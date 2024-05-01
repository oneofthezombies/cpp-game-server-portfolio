#include "actor_system.h"

#include "kero/core/utils.h"
#include "kero/log/log_builder.h"

using namespace kero;

static const std::string kShutdown = "shutdown";

kero::Mail::Mail(std::string &&from,
                 std::string &&to,
                 std::string &&event,
                 Dict &&body) noexcept
    : from{std::move(from)},
      to{std::move(to)},
      event{std::move(event)},
      body{std::move(body)} {}

auto
kero::Mail::Clone() const noexcept -> Mail {
  return Mail{std::string{from},
              std::string{to},
              std::string{event},
              body.Clone()};
}

kero::MailBox::MailBox(std::string &&name,
                       spsc::Tx<Mail> &&tx,
                       spsc::Rx<Mail> &&rx) noexcept
    : name{std::move(name)}, tx{std::move(tx)}, rx{std::move(rx)} {}

auto
kero::ActorSystem::CreateMailBox(const std::string &name) noexcept
    -> Result<MailBox> {
  using ResultT = Result<MailBox>;

  if (auto res = ValidateName(name); res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  std::lock_guard lock{mutex_};
  if (mail_boxes_.find(name) != mail_boxes_.end()) {
    return ResultT::Err(Dict{}
                            .Set("message", "mailbox name already exists")
                            .Set("name", name)
                            .Take());
  }

  auto [from_actor_tx, to_system_rx] = spsc::Channel<Mail>::Builder{}.Build();
  auto [from_system_tx, to_actor_rx] = spsc::Channel<Mail>::Builder{}.Build();

  mail_boxes_.try_emplace(name,
                          MailBox{std::string{name},
                                  spsc::Tx<Mail>{std::move(from_system_tx)},
                                  spsc::Rx<Mail>{std::move(to_system_rx)}});

  return ResultT::Ok(MailBox{std::string{name},
                             spsc::Tx<Mail>{std::move(from_actor_tx)},
                             spsc::Rx<Mail>{std::move(to_actor_rx)}});
}

auto
kero::ActorSystem::DestroyMailBox(const std::string &name) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  std::lock_guard lock{mutex_};
  if (mail_boxes_.find(name) == mail_boxes_.end()) {
    return ResultT::Err(Dict{}
                            .Set("message", "mailbox name not found")
                            .Set("name", name)
                            .Take());
  }

  mail_boxes_.erase(name);
  return OkVoid();
}

auto
kero::ActorSystem::ValidateName(const std::string &name) const noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (name.empty()) {
    return ResultT::Err(Dict{}.Set("message", "name is empty").Take());
  }

  if (name.size() > kMaxNameLength) {
    return ResultT::Err(
        Dict{}
            .Set("message", "name is too long")
            .Set("max_length", static_cast<double>(kMaxNameLength))
            .Set("length", static_cast<double>(name.size()))
            .Take());
  }

  if (name == "all") {
    return ResultT::Err(Dict{}.Set("message", "name is reserved").Take());
  }

  return OkVoid();
}

auto
kero::ActorSystem::Run(spsc::Rx<Dict> &&rx) -> Result<Void> {
  while (true) {
    if (auto message = rx.TryReceive(); message.IsSome()) {
      if (message.TakeUnwrap().Has(kShutdown)) {
        break;
      }
    }

    {
      std::lock_guard lock{mutex_};
      for (auto &[name, mail_box] : mail_boxes_) {
        auto mail = mail_box.rx.TryReceive();
        if (mail.IsNone()) {
          continue;
        }

        auto [from, to, event, body] = mail.TakeUnwrap();

        // broadcast
        if (to == "all") {
          for (auto &[name, other_mail_box] : mail_boxes_) {
            other_mail_box.tx.Send(Mail{std::string{from},
                                        std::string{name},
                                        std::string{event},
                                        body.Clone()});
          }

          continue;
        }

        // unicast
        auto it = mail_boxes_.find(to);
        if (it == mail_boxes_.end()) {
          log::Warn("Failed to find mail box")
              .Data("from", from)
              .Data("to", to)
              .Data("event", event)
              .Log();
          continue;
        }

        it->second.tx.Send(Mail{std::string{from},
                                std::string{to},
                                std::string{event},
                                body.Clone()});
      }
    }
  }

  return OkVoid();
}

kero::ThreadActorSystem::ThreadActorSystem(
    Pin<ActorSystem> actor_system) noexcept
    : actor_system_{actor_system} {}

auto
kero::ThreadActorSystem::Start() noexcept -> Result<Void> {
  if (thread_.joinable()) {
    return Error::From(Dict{}.Set("message", "thread already started").Take());
  }

  if (tx_ != nullptr) {
    return Error::From(Dict{}.Set("message", "tx already initialized").Take());
  }

  auto [tx, rx] = spsc::Channel<Dict>::Builder{}.Build();
  tx_ = std::make_unique<spsc::Tx<Dict>>(std::move(tx));

  thread_ = std::thread{ThreadMain, actor_system_, std::move(rx)};
  return OkVoid();
}

auto
kero::ThreadActorSystem::Stop() noexcept -> Result<Void> {
  if (!thread_.joinable()) {
    return Error::From(Dict{}.Set("message", "thread not started").Take());
  }

  if (tx_ == nullptr) {
    return Error::From(Dict{}.Set("message", "tx not initialized").Take());
  }

  tx_->Send(Dict{}.Set(std::string{kShutdown}, true).Take());
  thread_.join();
  return OkVoid();
}

auto
kero::ThreadActorSystem::ThreadMain(Pin<ActorSystem> actor_system,
                                    spsc::Rx<Dict> &&rx) -> void {
  if (auto res = actor_system.Unwrap().Run(std::move(rx))) {
    log::Info("Actor system finished").Log();
  } else {
    log::Error("Actor system failed").Data("error", res.TakeErr()).Log();
  }
}
