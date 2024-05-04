#include "actor_system.h"

#include "kero/core/utils.h"
#include "kero/log/log_builder.h"

using namespace kero;

static const std::string kShutdown = "shutdown";

kero::Mail::Mail(std::string &&from,
                 std::string &&to,
                 std::string &&event,
                 FlatJson &&body) noexcept
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
  if (mail_box_map_.find(name) != mail_box_map_.end()) {
    return ResultT::Err(FlatJson{}
                            .Set("message", "mailbox name already exists")
                            .Set("name", name)
                            .Take());
  }

  auto [from_actor_tx, to_system_rx] = spsc::Channel<Mail>::Builder{}.Build();
  auto [from_system_tx, to_actor_rx] = spsc::Channel<Mail>::Builder{}.Build();

  mail_box_map_.try_emplace(name,
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
  if (mail_box_map_.find(name) == mail_box_map_.end()) {
    return ResultT::Err(FlatJson{}
                            .Set("message", "mailbox name not found")
                            .Set("name", name)
                            .Take());
  }

  mail_box_map_.erase(name);
  return OkVoid();
}

auto
kero::ActorSystem::ValidateName(const std::string &name) const noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (name.empty()) {
    return ResultT::Err(FlatJson{}.Set("message", "name is empty").Take());
  }

  if (name.size() > kMaxNameLength) {
    return ResultT::Err(FlatJson{}
                            .Set("message", "name is too long")
                            .Set("max_length", kMaxNameLength)
                            .Set("length", name.size())
                            .Take());
  }

  if (name == "broadcast") {
    return ResultT::Err(
        FlatJson{}.Set("message", "broadcast is reserved").Take());
  }

  return OkVoid();
}

auto
kero::ActorSystem::Run(spsc::Rx<FlatJson> &&rx) -> Result<Void> {
  while (true) {
    if (auto message = rx.TryReceive()) {
      if (message.TakeUnwrap().Has(kShutdown)) {
        break;
      }
    }

    {
      std::lock_guard lock{mutex_};
      for (auto &[name, mail_box] : mail_box_map_) {
        auto mail = mail_box.rx.TryReceive();
        if (mail.IsNone()) {
          continue;
        }

        auto [from, to, event, body] = mail.TakeUnwrap();

        if (to == "broadcast") {
          for (auto &[name, other_mail_box] : mail_box_map_) {
            if (name == from) {
              continue;
            }

            other_mail_box.tx.Send(Mail{std::string{from},
                                        std::string{name},
                                        std::string{event},
                                        body.Clone()});
          }

          continue;
        }

        // unicast
        auto it = mail_box_map_.find(to);
        if (it == mail_box_map_.end()) {
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
    const Borrow<ActorSystem> actor_system) noexcept
    : actor_system_{actor_system} {}

auto
kero::ThreadActorSystem::Start() noexcept -> Result<Void> {
  if (thread_.joinable()) {
    return Error::From(
        FlatJson{}.Set("message", "thread already started").Take());
  }

  if (tx_ != nullptr) {
    return Error::From(
        FlatJson{}.Set("message", "tx already initialized").Take());
  }

  auto [tx, rx] = spsc::Channel<FlatJson>::Builder{}.Build();
  tx_ = std::make_unique<spsc::Tx<FlatJson>>(std::move(tx));

  thread_ = std::thread{ThreadMain, actor_system_, std::move(rx)};
  return OkVoid();
}

auto
kero::ThreadActorSystem::Stop() noexcept -> Result<Void> {
  if (!thread_.joinable()) {
    return Error::From(FlatJson{}.Set("message", "thread not started").Take());
  }

  if (tx_ == nullptr) {
    return Error::From(FlatJson{}.Set("message", "tx not initialized").Take());
  }

  tx_->Send(FlatJson{}.Set(std::string{kShutdown}, true).Take());
  thread_.join();
  return OkVoid();
}

auto
kero::ThreadActorSystem::ThreadMain(const Borrow<ActorSystem> actor_system,
                                    spsc::Rx<FlatJson> &&rx) -> void {
  if (auto res = actor_system->Run(std::move(rx))) {
    log::Info("Actor system finished").Log();
  } else {
    log::Error("Actor system failed").Data("error", res.TakeErr()).Log();
  }
}
