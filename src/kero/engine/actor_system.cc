#include "actor_system.h"

using namespace kero;

kero::Mail::Mail(std::string &&from, std::string &&to, Dict &&body) noexcept
    : from{std::move(from)}, to{std::move(to)}, body{std::move(body)} {}

auto
kero::Mail::Clone() const noexcept -> Mail {
  return Mail{std::string{from}, std::string{to}, body.Clone()};
}

kero::MailBox::MailBox(Tx<Mail> &&tx, Rx<Mail> &&rx) noexcept
    : tx{std::move(tx)}, rx{std::move(rx)} {}

kero::Actor::Actor(std::string &&name, MailBox &&mail_box) noexcept
    : Component{std::move(name)}, mail_box_{std::move(mail_box)} {}

auto
kero::Actor::OnUpdate(Engine &engine) noexcept -> void {
  auto mail = mail_box_.rx.TryReceive();
  if (mail.IsNone()) {
    return;
  }

  auto [from, to, body] = mail.TakeUnwrap();
  // TODO
}

kero::ActorSystem::ActorSystem(Tx<Dict> &&run_tx,
                               std::unique_ptr<Rx<Dict>> &&run_rx) noexcept
    : run_tx_{std::move(run_tx)}, run_rx_{std::move(run_rx)} {}

auto
kero::ActorSystem::Builder::Build() noexcept -> ActorSystem {
  auto [run_tx, run_rx] = Channel<Dict>::Builder{}.Build();
  return ActorSystem{
      std::move(run_tx),
      std::unique_ptr<Rx<Dict>>{new Rx<Dict>{std::move(run_rx)}}};
}

kero::ActorSystem::~ActorSystem() noexcept {
  if (IsRunning()) {
    [[maybe_unused]] auto stopped = Stop();
  }
}

auto
kero::ActorSystem::Start() noexcept -> Result<Void> {
  if (IsRunning()) {
    return Error::From(kAlreadyRunning);
  }

  if (!run_rx_) {
    return Error::From(kMultipleStartNotAllowed);
  }

  run_thread_ = std::thread{ThreadMain, std::move(run_rx_)};
  return Void{};
}

auto
kero::ActorSystem::Stop() noexcept -> bool {
  if (!IsRunning()) {
    return false;
  }

  run_tx_.Send(Dict{}.Set("shutdown", true).Take());
  run_thread_.join();
  return true;
}

auto
kero::ActorSystem::IsRunning() const noexcept -> bool {
  return run_thread_.joinable();
}

auto
kero::ActorSystem::CreateActor(std::string &&name) noexcept
    -> Result<ActorPtr> {
  if (auto res = ValidateName(name); res.IsErr()) {
    return Result<ActorPtr>{Error::From(res.TakeErr())};
  }

  std::lock_guard lock{mutex_};
  if (mail_boxes_.find(name) != mail_boxes_.end()) {
    return Result<ActorPtr>{
        Error::From(kMailBoxNameAlreadyExists,
                    Dict{}.Set("name", std::string{name}).Take())};
  }

  auto [from_actor_tx, to_system_rx] = Channel<Mail>::Builder{}.Build();
  auto [from_system_tx, to_actor_rx] = Channel<Mail>::Builder{}.Build();

  mail_boxes_.try_emplace(std::move(name),
                          MailBox{Tx<Mail>{std::move(from_system_tx)},
                                  Rx<Mail>{std::move(to_system_rx)}});

  return Result<ActorPtr>{std::make_unique<Actor>(
      std::move(name),
      MailBox{std::move(from_actor_tx), std::move(to_actor_rx)})};
}

auto
kero::ActorSystem::DeleteMailBox(const std::string &name) noexcept -> bool {
  std::lock_guard lock{mutex_};
  if (mail_boxes_.find(name) == mail_boxes_.end()) {
    return false;
  }

  mail_boxes_.erase(name);
  return true;
}

auto
kero::ActorSystem::ValidateName(const std::string &name) const noexcept
    -> Result<Void> {
  if (name.empty()) {
    return Error::From(kEmptyNameNotAllowed,
                       Dict{}.Set("name", std::string{name}).Take());
  }

  if (name.size() > kMaxNameLength) {
    return Error::From(kNameTooLong,
                       Dict{}.Set("name", std::string{name}).Take());
  }

  if (name == "all") {
    return Error::From(kReservedNameNotAllowed,
                       Dict{}.Set("name", std::string{name}).Take());
  }

  return Void{};
}

auto
kero::ActorSystem::ThreadMain(ActorSystem &self,
                              std::unique_ptr<Rx<Dict>> &&run_rx) -> void {
  while (true) {
    if (auto message = run_rx->TryReceive(); message.IsSome()) {
      if (message.TakeUnwrap().Has("shutdown")) {
        break;
      }
    }

    {
      std::lock_guard lock{self.mutex_};
      for (auto &[name, mail_box] : self.mail_boxes_) {
        auto mail = mail_box.rx.TryReceive();
        if (mail.IsNone()) {
          continue;
        }

        auto [from, to, body] = mail.TakeUnwrap();

        // broadcast
        if (to == "all") {
          for (auto &[name, other_mail_box] : self.mail_boxes_) {
            if (name == from) {
              continue;
            }

            other_mail_box.tx.Send(
                Mail{std::string{from}, std::string{name}, body.Clone()});
          }

          continue;
        }

        // unicast
        auto it = self.mail_boxes_.find(to);
        if (it == self.mail_boxes_.end()) {
          continue;
        }

        it->second.tx.Send(
            Mail{std::string{from}, std::string{to}, body.Clone()});
      }
    }
  }
}
