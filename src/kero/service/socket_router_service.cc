#include "socket_router_service.h"

#include "kero/engine/agent.h"
#include "kero/engine/constants.h"
#include "kero/log/log_builder.h"
#include "kero/service/actor_service.h"
#include "kero/service/config_service.h"

using namespace kero;

kero::SocketRouterService::SocketRouterService() noexcept
    : Service{ServiceKind::kRouter,
              {ServiceKind::kActor, ServiceKind::kConfig}} {}

auto
kero::SocketRouterService::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;
  if (!agent.SubscribeEvent(EventSocketOpen::kEvent, GetKind())) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message",
                 std::string{"Failed to subscribe to socket open event"})
            .Take()));
  }

  if (!agent.HasServiceIs<ActorService>(ServiceKind::kActor)) {
    return ResultT::Err(Error::From(
        Dict{}.Set("message", std::string{"ActorService not found"}).Take()));
  }

  auto config = agent.GetServiceAs<ConfigService>(ServiceKind::kConfig);
  if (!config) {
    return ResultT::Err(Error::From(
        Dict{}.Set("message", std::string{"ConfigService not found"}).Take()));
  }

  auto target_actor =
      config.Unwrap().GetConfig().GetOrDefault("target_actor", std::string{});
  if (target_actor.empty()) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message", std::string{"target_actor not found in config"})
            .Take()));
  }

  target_actor_ = std::move(target_actor);

  return ResultT::Ok(Void{});
}

auto
kero::SocketRouterService::OnEvent(Agent& agent,
                                   const std::string& event,
                                   const Dict& data) noexcept -> void {
  if (event != EventSocketOpen::kEvent) {
    return;
  }

  if (target_actor_.empty()) {
    log::Error("target_actor must be set for RouterService").Log();
    return;
  }

  agent.GetServiceAs<ActorService>(ServiceKind::kActor)
      .Unwrap()
      .SendMail(std::string{target_actor_}, std::string{event}, data.Clone());
}
