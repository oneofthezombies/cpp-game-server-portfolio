#include "service.h"

using namespace kero;

kero::Service::Service(const Kind kind) noexcept : kind_{kind} {}

auto
kero::Service::GetKind() const noexcept -> Kind {
  return kind_;
}

auto
kero::Service::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;
  return ResultT::Ok(Void{});
}

auto
kero::Service::OnDestroy(Agent& agent) noexcept -> void {
  // noop
}

auto
kero::Service::OnUpdate(Agent& agent) noexcept -> void {
  // noop
}

auto
kero::Service::OnEvent(Agent& agent,
                       const std::string& event,
                       const Dict& data) noexcept -> void {
  // noop
}
