#ifndef KERO_MIDDLEWARE_SOCKET_POOL_SERVICE_H
#define KERO_MIDDLEWARE_SOCKET_POOL_SERVICE_H

#include <unordered_set>

#include "kero/core/common.h"
#include "kero/core/utils.h"
#include "kero/engine/service.h"
#include "kero/engine/service_kind.h"
#include "kero/log/log_builder.h"
#include "kero/middleware/common.h"
#include "kero/middleware/io_event_loop_service.h"

namespace kero {

template <typename T>
class SocketPoolService : public Service {
 public:
  using EventHandler = std::function<Result<Void>(T*, const FlatJson&)>;

  explicit SocketPoolService(
      const Pin<RunnerContext> runner_context,
      DependencyDeclarations&& dependency_declarations) noexcept
      : Service{runner_context, {kServiceKindId_IoEventLoop}} {
    for (const auto dependency : dependency_declarations) {
      if (dependency == kServiceKindId_IoEventLoop) {
        continue;
      }

      dependency_declarations_.push_back(dependency);
    }
  }

  virtual ~SocketPoolService() noexcept override = default;
  CLASS_KIND_MOVABLE(SocketPoolService);

  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void> override {
    using ResultT = Result<Void>;

    if (!SubscribeEvent(EventSocketOpen::kEvent)) {
      return ResultT::Err(Error::From(
          FlatJson{}
              .Set("message",
                   std::string{"Failed to subscribe to socket open event"})
              .Take()));
    }

    if (!SubscribeEvent(EventSocketClose::kEvent)) {
      return ResultT::Err(Error::From(
          FlatJson{}
              .Set("message",
                   std::string{"Failed to subscribe to socket close event"})
              .Take()));
    }

    return OkVoid();
  }

  virtual auto
  OnDestroy() noexcept -> void override {
    if (!UnsubscribeEvent(EventSocketOpen::kEvent)) {
      log::Error("Failed to unsubscribe from socket open event").Log();
    }

    if (!UnsubscribeEvent(EventSocketClose::kEvent)) {
      log::Error("Failed to unsubscribe from socket close event").Log();
    }
  }

  virtual auto
  OnEvent(const std::string& event,
          const FlatJson& data) noexcept -> void override {
    auto found = event_handler_map_.find(event);
    if (found == event_handler_map_.end()) {
      log::Error("Event handler not found for event")
          .Data("event", event)
          .Log();
      return;
    }

    if (auto res = found->second(static_cast<T*>(this), data); res.IsErr()) {
      log::Error("Failed to handle event")
          .Data("event", event)
          .Data("error", res.TakeErr())
          .Log();
    }
  }

  [[nodiscard]] auto
  RegisterSocket(const SocketId socket_id) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    if (auto res = GetDependency<IoEventLoopService>()->AddFd(
            socket_id,
            {.in = true, .edge_trigger = true});
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    socket_ids_.insert(socket_id);
    return OkVoid();
  }

  [[nodiscard]] auto
  UnregisterSocket(const SocketId socket_id) noexcept -> Result<Void> {
    if (socket_ids_.erase(socket_id) == 0) {
      log::Error("Failed to remove socket_id from set")
          .Data("socket_id", socket_id)
          .Log();
    }

    if (auto res = GetDependency<IoEventLoopService>()->RemoveFd(socket_id);
        res.IsErr()) {
      log::Error("Failed to remove socket_id from epoll")
          .Data("socket_id", socket_id)
          .Log();
    }

    return OkVoid();
  }

  auto
  RegisterEventHandler(const std::string& event,
                       EventHandler&& handler) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    auto found = event_handler_map_.find(event);
    if (found != event_handler_map_.end()) {
      log::Error("Event handler already registered for event")
          .Data("event", event)
          .Log();
      return ResultT::Err(
          FlatJson{}
              .Set("message", "Event handler already registered for event")
              .Set("event", event)
              .Take());
    }

    event_handler_map_.emplace(event, std::move(handler));
    return OkVoid();
  }

 protected:
  std::unordered_set<SocketId> socket_ids_;

 private:
  std::unordered_map<std::string /* event */, EventHandler> event_handler_map_;
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_SOCKET_POOL_SERVICE_H
