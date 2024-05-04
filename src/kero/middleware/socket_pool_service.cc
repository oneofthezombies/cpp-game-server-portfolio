#include "socket_pool_service.h"

#include "kero/core/utils.h"
#include "kero/engine/runner_context.h"
#include "kero/log/log_builder.h"
#include "kero/middleware/common.h"
#include "kero/middleware/io_event_loop_service.h"

using namespace kero;

auto
kero::SocketPoolService::GetKindId() noexcept -> ServiceKindId {
  return kServiceKindId_SocketPool;
}

auto
kero::SocketPoolService::GetKindName() noexcept -> ServiceKindName {
  return "socket_pool";
}

kero::SocketPoolService::SocketPoolService(
    const Pin<RunnerContext> runner_context) noexcept
    : Service{runner_context, {kServiceKindId_IoEventLoop}} {}

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
kero::SocketPoolService::RegisterSocket(const SocketId socket_id) noexcept
    -> Result<Void> {
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

auto
kero::SocketPoolService::UnregisterSocket(
    const SocketId socket_id, std::string&& reason) noexcept -> Result<Void> {
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
