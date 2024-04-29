#include "router_service.h"

#include "kero/engine/actor_system.h"
#include "kero/engine/agent.h"
#include "kero/engine/config_service.h"
#include "kero/engine/constants.h"
#include "kero/log/log_builder.h"

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

  if (!agent.HasServiceIs<ActorService>(ServiceKind::kActor)) {
    return ResultT::Err(Error::From(
        Dict{}.Set("message", std::string{"ActorService not found"}).Take()));
  }

  auto config = agent.GetServiceAs<ConfigService>(ServiceKind::kConfig);
  if (!config) {
    return ResultT::Err(Error::From(
        Dict{}.Set("message", std::string{"ConfigService not found"}).Take()));
  }

  auto routing_target_actor =
      config.Unwrap().GetConfig().GetOrDefault("routing_target_actor",
                                               std::string{});
  if (routing_target_actor.empty()) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message",
                 std::string{"routing_target_actor not found in config"})
            .Take()));
  }

  routing_target_actor_ = std::move(routing_target_actor);

  return ResultT::Ok(Void{});
}

auto
kero::RouterService::OnEvent(Agent& agent,
                             const std::string& event,
                             const Dict& data) noexcept -> void {
  if (event != EventSocketOpen::kEvent) {
    return;
  }

  if (routing_target_actor_.empty()) {
    log::Error("routing_target_actor must be set for RouterService").Log();
    return;
  }

  agent.GetServiceAs<ActorService>(ServiceKind::kActor)
      .Unwrap()
      .SendMail(std::string{routing_target_actor_},
                std::string{event},
                data.Clone());
}
