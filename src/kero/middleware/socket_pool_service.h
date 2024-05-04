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
  using MethodEventHandler =
      std::function<Result<Void>(T* self, const FlatJson&)>;
  using EventHandler = std::function<Result<Void>(const FlatJson&)>;

  explicit SocketPoolService(
      const Borrow<RunnerContext> runner_context,
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
  KERO_CLASS_KIND_MOVABLE(SocketPoolService);

  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void> override {
    using ResultT = Result<Void>;
    return OkVoid();
  }

  virtual auto
  OnDestroy() noexcept -> void override {
    for (const auto& [event, _] : event_handler_map_) {
      if (auto res = UnsubscribeEvent(event); res.IsErr()) {
        log::Error("Failed to unsubscribe event")
            .Data("event", event)
            .Data("error", res.TakeErr())
            .Log();
      }
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

    if (auto res = found->second(data); res.IsErr()) {
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

  [[nodiscard]] auto
  RegisterMethodEventHandler(const std::string& event,
                             MethodEventHandler&& handler) noexcept
      -> Result<Void> {
    using ResultT = Result<Void>;

    if (auto res = SubscribeEvent(event); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    auto found = event_handler_map_.find(event);
    if (found != event_handler_map_.end()) {
      return ResultT::Err(
          FlatJson{}
              .Set("message", "Event handler already registered for event")
              .Set("event", event)
              .Take());
    }

    event_handler_map_.emplace(
        event,
        std::bind(handler, static_cast<T*>(this), std::placeholders::_1));

    return OkVoid();
  }

 protected:
  std::unordered_set<SocketId> socket_ids_;

 private:
  std::unordered_map<std::string /* event */, EventHandler> event_handler_map_;
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_SOCKET_POOL_SERVICE_H
