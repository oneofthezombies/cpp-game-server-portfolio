#include "common.h"
#include "kero/core/flat_json.h"
#include "kero/core/flat_json_parser.h"
#include "kero/core/utils.h"
#include "kero/engine/actor_service.h"
#include "kero/engine/common.h"
#include "kero/engine/runner_context.h"
#include "kero/log/log_builder.h"
#include "kero/middleware/common.h"
#include "kero/middleware/io_event_loop_service.h"
#include "kero/middleware/socket_pool_service.h"

using namespace kero;

class MatchService final : public SocketPoolService {
 public:
  static auto
  GetKindId() noexcept -> ServiceKindId {
    return kServiceKindIdMatch;
  }

  static auto
  GetKindName() noexcept -> ServiceKindName {
    return "match";
  }

  explicit MatchService(const Pin<RunnerContext> runner_context) noexcept
      : Service{runner_context,
                {kServiceKindIdSocketPool,
                 kServiceKindIdIoEventLoop,
                 kServiceKindIdActor}} {}

  auto
  OnCreate() noexcept -> Result<Void> override {
    using ResultT = Result<Void>;

    if (!SubscribeEvent(EventSocketRegister::kEvent)) {
      return ResultT::Err(Error::From(
          FlatJson{}
              .Set("message", "Failed to subscribe to socket register event")
              .Take()));
    }

    if (!SubscribeEvent(EventSocketUnregister::kEvent)) {
      return ResultT::Err(Error::From(
          FlatJson{}
              .Set("message", "Failed to subscribe to socket unregister event")
              .Take()));
    }

    return OkVoid();
  }

  auto
  OnDestroy() noexcept -> void override {
    if (!UnsubscribeEvent(EventSocketRegister::kEvent)) {
      log::Error("Failed to unsubscribe from socket register event").Log();
    }

    if (!UnsubscribeEvent(EventSocketUnregister::kEvent)) {
      log::Error("Failed to unsubscribe from socket unregister event").Log();
    }
  }

  auto
  OnEvent(const std::string &event,
          const FlatJson &data) noexcept -> void override {
    if (event == EventSocketRegister::kEvent) {
      OnSocketRegister(data);
    } else if (event == EventSocketUnregister::kEvent) {
      OnSocketUnregister(data);
    } else {
      log::Error("Unknown event").Data("event", event).Log();
    }
  }

 private:
  auto
  OnSocketRegister(const FlatJson &data) noexcept -> void {
    const auto fd = data.TryGet<u64>(EventSocketRegister::kFd);
    if (!fd) {
      log::Error("Failed to get fd from socket register event")
          .Data("data", data)
          .Log();
      return;
    }

    auto stringified = FlatJsonStringifier{}.Stringify(
        FlatJson{}.Set("kind", "connect").Set("fd", fd.Unwrap()).Take());
    if (stringified.IsErr()) {
      log::Error("Failed to stringify connect message")
          .Data("error", stringified.TakeErr())
          .Log();
      return;
    }

    if (auto res = GetDependency<IoEventLoopService>()->WriteToFd(
            fd.Unwrap(),
            stringified.TakeOk());
        res.IsErr()) {
      log::Error("Failed to send connect message to socket")
          .Data("fd", fd.Unwrap())
          .Data("error", res.TakeErr())
          .Log();
      return;
    }

    const auto count = data.TryGet<u64>(EventSocketRegister::kCount);
    if (!count) {
      log::Error("Failed to get count from socket register event")
          .Data("data", data)
          .Log();
      return;
    }

    if (count.Unwrap() >= 2) {
      auto socket_pool_service = GetDependency<SocketPoolService>();
      auto &sockets = socket_pool_service->GetSockets();
      const auto player1_it = sockets.begin();
      const auto player1 = *player1_it;
      const auto player2_it = std::next(player1_it);
      const auto player2 = *player2_it;
      if (auto res = socket_pool_service->UnregisterSocket(player1, "match");
          res.IsErr()) {
        log::Error("Failed to unregister player1 socket")
            .Data("fd", player1)
            .Data("error", res.TakeErr())
            .Log();
        return;
      }

      if (auto res = socket_pool_service->UnregisterSocket(player2, "match");
          res.IsErr()) {
        log::Error("Failed to unregister player2 socket")
            .Data("fd", player2)
            .Data("error", res.TakeErr())
            .Log();
        return;
      }
    }
  }

  auto
  OnSocketUnregister(const FlatJson &data) noexcept -> void {
    const auto fd = data.TryGet<u64>(EventSocketUnregister::kFd);
    if (!fd) {
      log::Error("Failed to get fd from socket unregister event")
          .Data("data", data)
          .Log();
      return;
    }

    const auto reason =
        data.TryGet<std::string>(EventSocketUnregister::kReason);
    if (!reason) {
      log::Error("Failed to get reason from socket unregister event")
          .Data("data", data)
          .Log();
      return;
    }

    if (reason.Unwrap() == "match") {
      GetDependency<ActorService>()->SendMail(
          "battle",
          EventSocketOpen::kEvent,
          FlatJson{}.Set(EventSocketOpen::kFd, fd.Unwrap()).Take());
    } else {
      log::Error("Unhandle reason").Data("reason", reason.Unwrap()).Log();
    }
  }
};
