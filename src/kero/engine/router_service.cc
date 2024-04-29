#include "router_service.h"

#include "kero/engine/actor_system.h"
#include "kero/engine/agent.h"
#include "kero/engine/config_service.h"
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

  if (!agent.HasServiceIs<ActorService>(ServiceKind::kActor)) {
    return ResultT::Err(Error::From(
        Dict{}.Set("message", std::string{"ActorService not found"}).Take()));
  }

  auto config = agent.GetServiceAs<ConfigService>(ServiceKind::kConfig);
  if (!config) {
    return ResultT::Err(Error::From(
        Dict{}.Set("message", std::string{"ConfigService not found"}).Take()));
  }

  auto root_actor =
      config.Unwrap().GetConfig().GetOrDefault("root_actor", std::string{});
  if (root_actor.empty()) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message", std::string{"root_actor not found in config"})
            .Take()));
  }

  root_actor_ = std::move(root_actor);

  return ResultT::Ok(Void{});
}

auto
kero::RouterService::OnEvent(Agent& agent,
                             const std::string& event,
                             const Dict& data) noexcept -> void {
  if (event != EventSocketOpen::kEvent) {
    return;
  }

  if (root_actor_.empty()) {
    // TODO: log error
    return;
  }

  agent.GetServiceAs<ActorService>(ServiceKind::kActor)
      .Unwrap()
      .SendMail(std::string{root_actor_}, data.Clone());
}
