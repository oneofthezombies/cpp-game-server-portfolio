#include "socket_router_service.h"

#include "kero/core/utils.h"
#include "kero/engine/actor_service.h"
#include "kero/engine/constants.h"
#include "kero/engine/runner_context.h"
#include "kero/log/log_builder.h"
#include "kero/middleware/config_service.h"
#include "kero/middleware/constants.h"

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
    const Pin<RunnerContext> runner_context) noexcept
    : Service{runner_context, {kServiceKindIdActor, kServiceKindIdConfig}} {}

auto
kero::SocketRouterService::OnCreate() noexcept -> Result<Void> {
  using ResultT = Result<Void>;
  if (!SubscribeEvent(EventSocketOpen::kEvent)) {
    return ResultT::Err(Error::From(
        FlatJson{}
            .Set("message",
                 std::string{"Failed to subscribe to socket open event"})
            .Take()));
  }

  auto target_actor =
      GetDependency<ConfigService>()->GetConfig().TryGet<std::string>(
          "target_actor");
  if (target_actor.IsNone()) {
    return ResultT::Err(Error::From(
        FlatJson{}
            .Set("message", std::string{"target_actor not found in config"})
            .Take()));
  }

  target_actor_ = target_actor.Unwrap();

  return OkVoid();
}

auto
kero::SocketRouterService::OnEvent(const std::string& event,
                                   const FlatJson& data) noexcept -> void {
  if (event != EventSocketOpen::kEvent) {
    return;
  }

  if (target_actor_.empty()) {
    log::Error("target_actor must be set for RouterService").Log();
    return;
  }

  GetDependency<ActorService>()->SendMail(std::string{target_actor_},
                                          std::string{event},
                                          data.Clone());
}
