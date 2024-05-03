#include "socket_router_service.h"

#include "kero/engine/actor_service.h"
#include "kero/engine/runner_context.h"
#include "kero/log/log_builder.h"
#include "kero/service/config_service.h"
#include "kero/service/constants.h"

using namespace kero;

kero::SocketRouterService::SocketRouterService(
    const Pin<RunnerContext> runner_context) noexcept
    : Service{runner_context,
              kServiceKindSocketRouter,
              {kServiceKindActor, kServiceKindConfig}} {}

auto
kero::SocketRouterService::OnCreate() noexcept -> Result<Void> {
  using ResultT = Result<Void>;
  if (!SubscribeEvent(EventSocketOpen::kEvent)) {
    return ResultT::Err(Error::From(
        Json{}
            .Set("message",
                 std::string{"Failed to subscribe to socket open event"})
            .Take()));
  }

  auto config = GetRunnerContext()
                    .GetService(kServiceKindConfig.id)
                    .Unwrap()
                    .As<ConfigService>(kServiceKindConfig.id);
  if (!config) {
    return ResultT::Err(Error::From(
        Json{}.Set("message", std::string{"ConfigService not found"}).Take()));
  }

  auto target_actor =
      config.Unwrap().GetConfig().GetOrDefault("target_actor", std::string{});
  if (target_actor.empty()) {
    return ResultT::Err(Error::From(
        Json{}
            .Set("message", std::string{"target_actor not found in config"})
            .Take()));
  }

  target_actor_ = std::move(target_actor);

  return OkVoid();
}

auto
kero::SocketRouterService::OnEvent(const std::string& event,
                                   const Json& data) noexcept -> void {
  if (event != EventSocketOpen::kEvent) {
    return;
  }

  if (target_actor_.empty()) {
    log::Error("target_actor must be set for RouterService").Log();
    return;
  }

  GetRunnerContext()
      .GetService(kServiceKindActor.id)
      .Unwrap()
      .As<ActorService>(kServiceKindActor.id)
      .Unwrap()
      .SendMail(std::string{target_actor_}, std::string{event}, data.Clone());
}
