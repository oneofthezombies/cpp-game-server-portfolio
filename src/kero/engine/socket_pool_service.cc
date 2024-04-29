#include "socket_pool_service.h"

#include "kero/engine/agent.h"
#include "kero/engine/constants.h"

using namespace kero;

kero::SocketPoolService::SocketPoolService() noexcept
    : Service{ServiceKind::kSocket} {}

auto
kero::SocketPoolService::OnCreate(Agent& agent) noexcept -> Result<Void> {
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
kero::SocketPoolService::OnEvent(Agent& agent,
                                 const std::string& event,
                                 const Dict& data) noexcept -> void {
  if (event != EventSocketOpen::kEvent) {
    return;
  }

  auto fd = data.GetOrDefault<int64_t>(EventSocketOpen::kFd, -1);
  if (fd == -1) {
    // TODO: log error
    return;
  }
}
