#include "component.h"

using namespace kero;

kero::Component::Component(std::string&& name) noexcept
    : name_{std::move(name)} {}

auto
kero::Component::GetName() const noexcept -> const std::string& {
  return name_;
}

auto
kero::Component::OnCreate(Engine& engine) noexcept -> void {
  // noop
}

auto
kero::Component::OnDestroy(Engine& engine) noexcept -> void {
  // noop
}

auto
kero::Component::OnUpdate(Engine& engine) noexcept -> void {
  // noop
}

auto
kero::Component::OnEvent(Engine& engine,
                         const std::string& event,
                         const Dict& data) noexcept -> void {
  // noop
}
