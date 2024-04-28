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

kero::Actor::Actor(std::string &&name) noexcept : Component{std::move(name)} {}

kero::ActorSystem::~ActorSystem() noexcept {
  if (IsRunning()) {
    [[maybe_unused]] auto stopped = Stop();
  }
}

auto
kero::ActorSystem::Start() noexcept -> bool {
  if (IsRunning()) {
    return false;
  }

  thread_ = std::thread{[] {
    while (true) {
      // Do something.
    }
  }};
  return true;
}

auto
kero::ActorSystem::Stop() noexcept -> bool {
  if (!IsRunning()) {
    return false;
  }

  thread_.join();
  return true;
}

auto
kero::ActorSystem::IsRunning() const noexcept -> bool {
  return thread_.joinable();
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

  return Void{};
}
