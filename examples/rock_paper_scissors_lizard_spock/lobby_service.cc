#include "kero/engine/agent.h"
#include "kero/engine/constants.h"
#include "kero/log/log_builder.h"
#include "kero/service/service.h"
#include "kero/service/socket_pool_service.h"

class LobbyService final : public kero::Service {
 public:
  explicit LobbyService() noexcept
      : Service{100, {kero::ServiceKind::kSocketPool}} {}

  auto
  OnCreate(kero::Agent &agent) noexcept -> kero::Result<kero::Void> override {
    using ResultT = kero::Result<kero::Void>;

    if (!agent.SubscribeEvent(kero::EventSocketRegister::kEvent, GetKind())) {
      return ResultT::Err(kero::Error::From(
          kero::Dict{}
              .Set("message", "Failed to subscribe to socket register event")
              .Take()));
    }

    if (!agent.SubscribeEvent(kero::EventSocketUnregister::kEvent, GetKind())) {
      return ResultT::Err(kero::Error::From(
          kero::Dict{}
              .Set("message", "Failed to subscribe to socket unregister event")
              .Take()));
    }

    return ResultT::Ok(kero::Void{});
  }

  auto
  OnDestroy(kero::Agent &agent) noexcept -> void override {
    if (!agent.UnsubscribeEvent(kero::EventSocketRegister::kEvent, GetKind())) {
      kero::log::Error("Failed to unsubscribe from socket register event")
          .Log();
    }

    if (!agent.UnsubscribeEvent(kero::EventSocketUnregister::kEvent,
                                GetKind())) {
      kero::log::Error("Failed to unsubscribe from socket unregister event")
          .Log();
    }
  }

  auto
  OnEvent(kero::Agent &agent,
          const std::string &event,
          const kero::Dict &data) noexcept -> void override {
    if (event == kero::EventSocketRegister::kEvent) {
      OnSocketRegister(agent, data);
    } else if (event == kero::EventSocketUnregister::kEvent) {
      OnSocketUnregister(agent, data);
    } else {
      kero::log::Error("Unknown event").Data("event", event).Log();
    }
  }

 private:
  auto
  OnSocketRegister(kero::Agent &agent, const kero::Dict &data) noexcept
      -> void {
    // TODO: Implement this method
  }

  auto
  OnSocketUnregister(kero::Agent &agent, const kero::Dict &data) noexcept
      -> void {
    // TODO: Implement this method
  }
};
