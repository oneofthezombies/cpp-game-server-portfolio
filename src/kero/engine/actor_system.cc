#include "actor_system.h"

#include "kero/engine/actor_service.h"
#include "kero/log/log_builder.h"

using namespace kero;

static constexpr std::string_view kShutdown = "shutdown";

kero::ActorSystem::ActorSystem() noexcept
    : run_channel_{spsc::Channel<Dict>::Builder{}.Build()} {}

auto
kero::ActorSystem::Builder::Build() noexcept -> ActorSystemPtr {
  return std::make_shared<ActorSystem>();
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

  run_thread_ = std::thread{ThreadMain, shared_from_this()};
  return Void{};
}

auto
kero::ActorSystem::Stop() noexcept -> bool {
  if (!IsRunning()) {
    return false;
  }

  run_channel_.tx.Send(Dict{}.Set(std::string{kShutdown}, true).Take());
  run_thread_.join();
  return true;
}

auto
kero::ActorSystem::IsRunning() const noexcept -> bool {
  return run_thread_.joinable();
}

auto
kero::ActorSystem::CreateActorService(std::string &&name) noexcept
    -> Result<ActorServicePtr> {
  if (auto res = ValidateName(name); res.IsErr()) {
    return Result<ActorServicePtr>{Error::From(res.TakeErr())};
  }

  std::lock_guard lock{mutex_};
  if (mail_boxes_.find(name) != mail_boxes_.end()) {
    return Result<ActorServicePtr>{
        Error::From(kMailBoxNameAlreadyExists,
                    Dict{}.Set("name", name).Take())};
  }

  auto [from_actor_tx, to_system_rx] = spsc::Channel<Mail>::Builder{}.Build();
  auto [from_system_tx, to_actor_rx] = spsc::Channel<Mail>::Builder{}.Build();

  mail_boxes_.try_emplace(name,
                          MailBox{spsc::Tx<Mail>{std::move(from_system_tx)},
                                  spsc::Rx<Mail>{std::move(to_system_rx)}});

  return Result<ActorServicePtr>{std::unique_ptr<ActorService>{new ActorService{
      std::move(name),
      MailBox{std::move(from_actor_tx), std::move(to_actor_rx)}}}};
}

auto
kero::ActorSystem::DestroyMailBox(const std::string &name) noexcept -> bool {
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
kero::ActorSystem::ThreadMain(ActorSystemPtr self) -> void {
  while (true) {
    if (auto message = self->run_channel_.rx.TryReceive(); message.IsSome()) {
      if (message.TakeUnwrap().Has(std::string{kShutdown})) {
        break;
      }
    }

    {
      std::lock_guard lock{self->mutex_};
      for (auto &[name, mail_box] : self->mail_boxes_) {
        auto mail = mail_box.rx.TryReceive();
        if (mail.IsNone()) {
          continue;
        }

        auto [from, to, event, body] = mail.TakeUnwrap();

        // broadcast
        if (to == "all") {
          for (auto &[name, other_mail_box] : self->mail_boxes_) {
            other_mail_box.tx.Send(Mail{std::string{from},
                                        std::string{name},
                                        std::string{event},
                                        body.Clone()});
          }

          continue;
        }

        // unicast
        auto it = self->mail_boxes_.find(to);
        if (it == self->mail_boxes_.end()) {
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
}
