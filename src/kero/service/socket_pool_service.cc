#include "socket_pool_service.h"

#include "io_event_loop_service.h"
#include "kero/engine/agent.h"
#include "kero/engine/constants.h"
#include "kero/log/log_builder.h"

using namespace kero;

kero::SocketPoolService::SocketPoolService() noexcept
    : Service{ServiceKind::kSocketPool, {ServiceKind::kIoEventLoop}} {}

auto
kero::SocketPoolService::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (!agent.SubscribeEvent(EventSocketOpen::kEvent, GetKind())) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message",
                 std::string{"Failed to subscribe to socket open event"})
            .Take()));
  }

  if (!agent.SubscribeEvent(EventSocketClose::kEvent, GetKind())) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message",
                 std::string{"Failed to subscribe to socket close event"})
            .Take()));
  }

  return ResultT::Ok(Void{});
}

auto
kero::SocketPoolService::OnEvent(Agent& agent,
                                 const std::string& event,
                                 const Dict& data) noexcept -> void {
  if (event == EventSocketOpen::kEvent) {
    OnSocketOpen(agent, data);
  } else if (event == EventSocketClose::kEvent) {
    OnSocketClose(agent, data);
  } else {
    log::Error("Unknown event").Data("event", event).Log();
  }
}

auto
kero::SocketPoolService::OnSocketOpen(Agent& agent, const Dict& data) noexcept
    -> void {
  auto fd = data.GetOrDefault<double>(EventSocketOpen::kFd, -1);
  if (fd == -1) {
    log::Error("Failed to get fd from event data").Log();
    return;
  }

  auto io_event_loop =
      agent.GetServiceAs<IoEventLoopService>(ServiceKind::kIoEventLoop);
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

  agent.Invoke(EventSocketRegister::kEvent,
               Dict{}.Set(EventSocketRegister::kFd, fd).Take());
}

auto
kero::SocketPoolService::OnSocketClose(Agent& agent, const Dict& data) noexcept
    -> void {
  auto fd = data.GetOrDefault<double>(EventSocketClose::kFd, -1);
  if (fd == -1) {
    log::Error("Failed to get fd from event data").Log();
    return;
  }

  sockets_.erase(fd);

  auto io_event_loop =
      agent.GetServiceAs<IoEventLoopService>(ServiceKind::kIoEventLoop);
  if (!io_event_loop) {
    log::Error("Failed to get IoEventLoopService").Log();
  } else {
    if (auto res = io_event_loop.Unwrap().RemoveFd(fd); res.IsErr()) {
      log::Error("Failed to remove fd from epoll").Data("fd", fd).Log();
    }
  }

  agent.Invoke(EventSocketUnregister::kEvent,
               Dict{}.Set(EventSocketUnregister::kFd, fd).Take());
}
