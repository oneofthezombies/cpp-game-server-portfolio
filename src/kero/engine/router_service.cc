#include "router_service.h"

#include "kero/engine/agent.h"
#include "kero/engine/constants.h"

using namespace kero;

kero::RouterService::RouterService() noexcept : Service{ServiceKind::kRouter} {}

auto
kero::RouterService::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;
  if (!agent.SubscribeEvent(EventSocketOpen::kEvent, GetKind())) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message",
                 std::string{"Failed to subscribe to socket open event"})
            .Take()));
  }

  return ResultT::Ok(Void{});
}

auto
kero::RouterService::OnEvent(Agent& agent,
                             const std::string& event,
                             const Dict& data) noexcept -> void {}
