#include "tcp_server_service.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include "kero/core/common.h"
#include "kero/core/utils.h"
#include "kero/engine/runner_context.h"
#include "kero/log/log_builder.h"
#include "kero/middleware/common.h"
#include "kero/middleware/config_service.h"
#include "kero/middleware/io_event_loop_service.h"

using namespace kero;

auto
kero::TcpServerService::GetKindId() noexcept -> ServiceKindId {
  return kServiceKindId_TcpServer;
}

auto
kero::TcpServerService::GetKindName() noexcept -> ServiceKindName {
  return "tcp_server";
}

kero::TcpServerService::TcpServerService(
    const Pin<RunnerContext> runner_context) noexcept
    : Service{runner_context,
              {kServiceKindId_Config, kServiceKindId_IoEventLoop}} {}

auto
kero::TcpServerService::OnCreate() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (!SubscribeEvent(EventSocketRead::kEvent)) {
    return ResultT::Err(Error::From(
        FlatJson{}
            .Set("message",
                 std::string{"Failed to subscribe to socket read event"})
            .Take()));
  }

  auto port_opt =
      GetDependency<ConfigService>()->GetConfig().TryGet<u16>("port");
  if (port_opt.IsNone()) {
    return ResultT::Err(
        Error::From(FlatJson{}
                        .Set("message", std::string{"port not found in config"})
                        .Take()));
  }

  const auto port = port_opt.TakeUnwrap();
  auto server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (!Fd::IsValid(server_fd)) {
    return ResultT::Err(Error::From(
        Errno::FromErrno()
            .IntoFlatJson()
            .Set("message", std::string{"Failed to create server socket"})
            .Take()));
  }

  int reuse{1};
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    return ResultT::Err(Error::From(
        Errno::FromErrno()
            .IntoFlatJson()
            .Set("message", std::string{"Failed to set socket reuse option"})
            .Take()));
  }

  if (auto res = Fd::UpdateNonBlocking(server_fd); res.IsErr()) {
    return ResultT::From(Error::From(res.TakeErr()));
  }

  struct sockaddr_in server_addr {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);
  if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) <
      0) {
    return ResultT::Err(Error::From(
        Errno::FromErrno()
            .IntoFlatJson()
            .Set("message", std::string{"Failed to bind server socket"})
            .Set("port", static_cast<double>(port))
            .Take()));
  }

  if (listen(server_fd, SOMAXCONN) < 0) {
    return ResultT::Err(Error::From(
        Errno::FromErrno()
            .IntoFlatJson()
            .Set("message", std::string{"Failed to listen on server"})
            .Take()));
  }

  if (auto res =
          GetDependency<IoEventLoopService>()->AddFd(server_fd, {.in = true});
      res.IsErr()) {
    return ResultT::Err(Error::From(res.TakeErr()));
  }

  server_fd_ = server_fd;
  return OkVoid();
}

auto
kero::TcpServerService::OnDestroy() noexcept -> void {
  if (!Fd::IsValid(server_fd_)) {
    return;
  }

  if (auto res = Fd::Close(server_fd_); res.IsErr()) {
    log::Error("Failed to close server fd")
        .Data("fd", server_fd_)
        .Data("error", res.TakeErr())
        .Log();
  }
}

auto
kero::TcpServerService::OnEvent(const std::string& event,
                                const FlatJson& data) noexcept -> void {
  if (event == EventSocketRead::kEvent) {
    auto socket_id = data.TryGet<u64>(EventSocketRead::kSocketId);
    if (socket_id.IsNone()) {
      log::Error("Failed to get fd from event data").Log();
      return;
    }

    const auto fd = static_cast<int>(socket_id.TakeUnwrap());
    if (fd == server_fd_) {
      struct sockaddr_in client_addr {};
      socklen_t addrlen = sizeof(struct sockaddr_in);
      auto client_fd =
          accept(server_fd_, (struct sockaddr*)&client_addr, &addrlen);
      if (!Fd::IsValid(client_fd)) {
        log::Error("Failed to accept client connection")
            .Data("server_fd", server_fd_)
            .Data("errno", Errno::FromErrno())
            .Log();
        return;
      }

      if (auto res = Fd::UpdateNonBlocking(client_fd); res.IsErr()) {
        log::Error("Failed to update client fd to non-blocking")
            .Data("client_fd", client_fd)
            .Data("error", res.TakeErr())
            .Log();
        return;
      }

      if (auto res = InvokeEvent(
              EventSocketOpen::kEvent,
              FlatJson{}
                  .Set(EventSocketOpen::kSocketId, static_cast<u64>(client_fd))
                  .Take())) {
        log::Error("Failed to invoke socket open event")
            .Data("error", res.TakeErr())
            .Log();
      }
    }
  }
}
