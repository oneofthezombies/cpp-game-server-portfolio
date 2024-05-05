#ifndef KERO_MIDDLEWARE_SOCKET_POOL_SERVICE_H
#define KERO_MIDDLEWARE_SOCKET_POOL_SERVICE_H

#include "kero/core/common.h"
#include "kero/core/flat_json_parser.h"
#include "kero/core/flat_json_scanner.h"
#include "kero/core/utils.h"
#include "kero/engine/service.h"
#include "kero/engine/service_kind.h"
#include "kero/log/log_builder.h"
#include "kero/middleware/common.h"
#include "kero/middleware/io_event_loop_service.h"

namespace kero {

struct SocketInfo {
  FlatJsonScanner scanner{};
};

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

  virtual auto
  OnCreate() noexcept -> Result<Void> override {
    using ResultT = Result<Void>;

    if (auto res = RegisterMethodEventHandler(EventSocketRead::kEvent,
                                              &SocketPoolService::OnSocketRead);
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

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
    if (auto res = InvokeMethodEvent(event, data); res.IsErr()) {
      log::Error("Failed to handle event")
          .Data("event", event)
          .Data("error", res.TakeErr())
          .Log();
    }
  }

  [[nodiscard]] auto
  OnSocketRead(const FlatJson& data) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    auto socket_id_opt = data.TryGet<u64>(EventSocketRead::kSocketId);
    if (!socket_id_opt) {
      return ResultT::Err(
          FlatJson{}
              .Set("message", "Failed to get socket id from data")
              .Take());
    }

    const auto socket_id = socket_id_opt.Unwrap();
    auto read_res = GetDependency<IoEventLoopService>()->ReadFromFd(socket_id);
    if (read_res.IsErr()) {
      auto err = read_res.TakeErr();
      if (err.code == IoEventLoopService::kSocketClosed) {
        return OkVoid();
      }

      return ResultT::Err(std::move(err));
    }

    auto found_socket_info = socket_map_.find(socket_id);
    if (found_socket_info == socket_map_.end()) {
      return ResultT::Err(
          FlatJson{}.Set("message", "Socket info not found").Take());
    }

    auto& socket_info = found_socket_info->second;
    socket_info.scanner.Push(read_res.TakeOk());
    auto token_opt = socket_info.scanner.Pop();
    if (!token_opt) {
      return OkVoid();
    }

    auto token = token_opt.Unwrap();
    auto parsed = FlatJsonParser{}.Parse(token);
    if (parsed.IsErr()) {
      return ResultT::Err(parsed.TakeErr());
    }

    auto read_data = parsed.TakeOk();
    auto event_opt = read_data.template TryGet<std::string>("__event");
    if (!event_opt) {
      return ResultT::Err(
          FlatJson{}.Set("message", "Failed to get event from data").Take());
    }

    (void)read_data.Set("__socket_id", socket_id);
    const auto event = event_opt.Unwrap();
    if (auto res = InvokeMethodEvent(event, read_data); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    return OkVoid();
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

    socket_map_.emplace(socket_id, SocketInfo{});
    return OkVoid();
  }

  [[nodiscard]] auto
  UnregisterSocket(const SocketId socket_id) noexcept -> Result<Void> {
    log::Debug("Unregistering socket").Data("socket_id", socket_id).Log();

    if (socket_map_.erase(socket_id) == 0) {
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

  [[nodiscard]] auto
  InvokeMethodEvent(const std::string& event,
                    const FlatJson& data) noexcept -> Result<Void> {
    auto found = event_handler_map_.find(event);
    if (found == event_handler_map_.end()) {
      return Result<Void>::Err(
          FlatJson{}.Set("message", "Event handler not found").Take());
    }

    if (auto res = found->second(data); res.IsErr()) {
      return Result<Void>::Err(res.TakeErr());
    }

    return OkVoid();
  }

 protected:
  std::unordered_map<SocketId, SocketInfo> socket_map_;

 private:
  std::unordered_map<std::string /* event */, EventHandler> event_handler_map_;
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_SOCKET_POOL_SERVICE_H
