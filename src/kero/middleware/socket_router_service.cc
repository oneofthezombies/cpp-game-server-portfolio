#include "socket_router_service.h"

#include "kero/core/utils.h"
#include "kero/engine/actor_service.h"
#include "kero/engine/common.h"
#include "kero/engine/runner_context.h"
#include "kero/middleware/common.h"

using namespace kero;

auto
kero::SocketRouterService::GetKindId() noexcept -> ServiceKindId {
  return kServiceKindIdSocketRouter;
}

auto
kero::SocketRouterService::GetKindName() noexcept -> ServiceKindName {
  return "socket_router";
}

kero::SocketRouterService::SocketRouterService(
    const Pin<RunnerContext> runner_context, std::string&& target) noexcept
    : Service{runner_context, {kServiceKindIdActor}},
      target_{std::move(target)} {}

auto
kero::SocketRouterService::OnCreate() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (target_.empty()) {
    return ResultT::Err(Error::From(
        FlatJson{}.Set("message", std::string{"target must be set"}).Take()));
  }

  if (!SubscribeEvent(EventSocketOpen::kEvent)) {
    return ResultT::Err(Error::From(
        FlatJson{}
            .Set("message",
                 std::string{"Failed to subscribe to socket open event"})
            .Take()));
  }

  return OkVoid();
}

auto
kero::SocketRouterService::OnEvent(const std::string& event,
                                   const FlatJson& data) noexcept -> void {
  if (event != EventSocketOpen::kEvent) {
    return;
  }

  GetDependency<ActorService>()->SendMail(std::string{target_},
                                          std::string{event},
                                          data.Clone());
}
