#include "socket_pool_service.h"

#include "constants.h"
#include "kero/core/utils.h"
#include "kero/engine/runner_context.h"
#include "kero/log/log_builder.h"
#include "kero/middleware/io_event_loop_service.h"

using namespace kero;

auto
kero::SocketPoolService::GetKindId() noexcept -> ServiceKindId {
  return kServiceKindIdSocketPool;
}

auto
kero::SocketPoolService::GetKindName() noexcept -> ServiceKindName {
  return "socket_pool";
}

kero::SocketPoolService::SocketPoolService(
    const Pin<RunnerContext> runner_context) noexcept
    : Service{runner_context, {kServiceKindIdIoEventLoop}} {}

auto
kero::SocketPoolService::OnCreate() noexcept -> Result<Void> {
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

auto
kero::SocketPoolService::OnEvent(const std::string& event,
                                 const FlatJson& data) noexcept -> void {
  if (event == EventSocketOpen::kEvent) {
    OnSocketOpen(data);
  } else if (event == EventSocketClose::kEvent) {
    OnSocketClose(data);
  } else {
    log::Error("Unknown event").Data("event", event).Log();
  }
}

auto
kero::SocketPoolService::OnSocketOpen(const FlatJson& data) noexcept -> void {
  auto fd_opt = data.TryGet<u64>(EventSocketOpen::kFd);
  if (fd_opt.IsNone()) {
    log::Error("Failed to get fd from event data").Log();
    return;
  }

  const auto fd = fd_opt.TakeUnwrap();
  auto io_event_loop = GetDependency<IoEventLoopService>();
  if (auto res = io_event_loop->AddFd(fd, {.in = true, .edge_trigger = true});
      res.IsErr()) {
    log::Error("Failed to add fd to epoll").Data("fd", fd).Log();
    return;
  }

  sockets_.insert(fd);

  InvokeEvent(EventSocketRegister::kEvent,
              FlatJson{}
                  .Set(EventSocketRegister::kFd, fd)
                  .Set(EventSocketRegister::kCount, sockets_.size())
                  .Take());
}

auto
kero::SocketPoolService::OnSocketClose(const FlatJson& data) noexcept -> void {
  auto fd_opt = data.TryGet<u64>(EventSocketClose::kFd);
  if (fd_opt.IsNone()) {
    log::Error("Failed to get fd from event data").Log();
    return;
  }

  const auto fd = fd_opt.TakeUnwrap();
  if (auto res = UnregisterSocket(fd, "close"); res.IsErr()) {
    log::Error("Failed to unregister socket").Data("fd", fd).Log();
  }
}

auto
kero::SocketPoolService::GetSockets() noexcept
    -> std::unordered_set<Fd::Value>& {
  return sockets_;
}

auto
kero::SocketPoolService::UnregisterSocket(
    const Fd::Value fd, std::string&& reason) noexcept -> Result<Void> {
  if (sockets_.erase(fd) == 0) {
    log::Error("Failed to remove fd from set").Data("fd", fd).Log();
  }

  if (auto res = GetDependency<IoEventLoopService>()->RemoveFd(fd);
      res.IsErr()) {
    log::Error("Failed to remove fd from epoll").Data("fd", fd).Log();
  }

  InvokeEvent(EventSocketUnregister::kEvent,
              FlatJson{}
                  .Set(EventSocketUnregister::kFd, fd)
                  .Set(EventSocketUnregister::kReason, std::move(reason))
                  .Take());

  return OkVoid();
}
