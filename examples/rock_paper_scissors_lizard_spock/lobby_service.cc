#include "kero/core/tiny_json.h"
#include "kero/engine/agent.h"
#include "kero/engine/constants.h"
#include "kero/log/log_builder.h"
#include "kero/service/io_event_loop_service.h"
#include "kero/service/service.h"
#include "kero/service/socket_pool_service.h"

class LobbyService final : public kero::Service {
 public:
  explicit LobbyService() noexcept
      : Service{100,
                {kero::ServiceKind::kSocketPool,
                 kero::ServiceKind::kIoEventLoop}} {}

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
    const auto fd = data.TryGetAsDouble(kero::EventSocketRegister::kFd);
    if (!fd) {
      kero::log::Error("Failed to get fd from socket register event")
          .Data("data", data)
          .Log();
      return;
    }

    const auto &io_event_loop = agent
                                    .GetServiceAs<kero::IoEventLoopService>(
                                        kero::ServiceKind::kIoEventLoop)
                                    .Unwrap();

    const auto data_to_send = kero::TinyJson::Stringify(
        kero::Dict{}
            .Set("kind", "connect")
            .Set("socket_id", std::to_string(fd.Unwrap()))
            .Take());
    if (auto res = io_event_loop.WriteToFd(static_cast<int>(fd.Unwrap()),
                                           data_to_send);
        res.IsErr()) {
      kero::log::Error("Failed to send connect message to socket")
          .Data("fd", fd.Unwrap())
          .Data("error", res.TakeErr())
          .Log();
    }
  }

  auto
  OnSocketUnregister(kero::Agent &agent, const kero::Dict &data) noexcept
      -> void {
    const auto fd = data.TryGetAsDouble(kero::EventSocketUnregister::kFd);
    if (!fd) {
      kero::log::Error("Failed to get fd from socket unregister event")
          .Data("data", data)
          .Log();
      return;
    }
  }
};
