#include "socket_pool_service.h"

#include "constants.h"
#include "kero/engine/runner_context.h"
#include "kero/log/log_builder.h"
#include "kero/service/io_event_loop_service.h"

using namespace kero;

kero::SocketPoolService::SocketPoolService(
    RunnerContextPtr&& runner_context) noexcept
    : Service{std::move(runner_context),
              kServiceKindSocketPool,
              {kServiceKindIoEventLoop}} {}

auto
kero::SocketPoolService::OnCreate() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (!SubscribeEvent(EventSocketOpen::kEvent)) {
    return ResultT::Err(Error::From(
        Json{}
            .Set("message",
                 std::string{"Failed to subscribe to socket open event"})
            .Take()));
  }

  if (!SubscribeEvent(EventSocketClose::kEvent)) {
    return ResultT::Err(Error::From(
        Json{}
            .Set("message",
                 std::string{"Failed to subscribe to socket close event"})
            .Take()));
  }

  return OkVoid();
}

auto
kero::SocketPoolService::OnEvent(const std::string& event,
                                 const Json& data) noexcept -> void {
  if (event == EventSocketOpen::kEvent) {
    OnSocketOpen(data);
  } else if (event == EventSocketClose::kEvent) {
    OnSocketClose(data);
  } else {
    log::Error("Unknown event").Data("event", event).Log();
  }
}

auto
kero::SocketPoolService::OnSocketOpen(const Json& data) noexcept -> void {
  auto fd = data.GetOrDefault<double>(EventSocketOpen::kFd, -1);
  if (fd == -1) {
    log::Error("Failed to get fd from event data").Log();
    return;
  }

  auto io_event_loop = GetRunnerContext()
                           .GetService(kServiceKindIoEventLoop.id)
                           .Unwrap()
                           .As<IoEventLoopService>(kServiceKindIoEventLoop.id);
  if (!io_event_loop) {
    log::Error("Failed to get IoEventLoopService").Log();
    return;
  }

  if (auto res =
          io_event_loop.Unwrap().AddFd(fd, {.in = true, .edge_trigger = true});
      res.IsErr()) {
    log::Error("Failed to add fd to epoll").Data("fd", fd).Log();
    return;
  }

  sockets_.insert(fd);

  InvokeEvent(EventSocketRegister::kEvent,
              Json{}.Set(EventSocketRegister::kFd, fd).Take());
}

auto
kero::SocketPoolService::OnSocketClose(const Json& data) noexcept -> void {
  auto fd = data.GetOrDefault<double>(EventSocketClose::kFd, -1);
  if (fd == -1) {
    log::Error("Failed to get fd from event data").Log();
    return;
  }

  sockets_.erase(fd);

  auto io_event_loop = GetRunnerContext()
                           .GetService(kServiceKindIoEventLoop.id)
                           .Unwrap()
                           .As<IoEventLoopService>(kServiceKindIoEventLoop.id);
  if (!io_event_loop) {
    log::Error("Failed to get IoEventLoopService").Log();
  } else {
    if (auto res = io_event_loop.Unwrap().RemoveFd(fd); res.IsErr()) {
      log::Error("Failed to remove fd from epoll").Data("fd", fd).Log();
    }
  }

  InvokeEvent(EventSocketUnregister::kEvent,
              Json{}.Set(EventSocketUnregister::kFd, fd).Take());
}
