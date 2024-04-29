#include "socket_service.h"

#include "kero/engine/agent.h"
#include "kero/engine/constants.h"

using namespace kero;

kero::SocketService::SocketService() noexcept : Service{ServiceKind::kSocket} {}

auto
kero::SocketService::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;
  // agent.SubscribeEvent(EventSocket::kEvent, GetKind());
  return ResultT::Ok(Void{});
}
