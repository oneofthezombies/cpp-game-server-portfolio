#include "component.h"

using namespace kero;

kero::Component::Component(const Kind kind) noexcept : kind_{kind} {}

auto
kero::Component::GetKind() const noexcept -> Kind {
  return kind_;
}

auto
kero::Component::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;
  return ResultT::Ok(Void{});
}

auto
kero::Component::OnDestroy(Agent& agent) noexcept -> void {
  // noop
}

auto
kero::Component::OnUpdate(Agent& agent) noexcept -> void {
  // noop
}

auto
kero::Component::OnEvent(Agent& agent,
                         const std::string& event,
                         const Dict& data) noexcept -> void {
  // noop
}
