#include "tcp_server_service.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include "kero/engine/agent.h"
#include "kero/engine/constants.h"
#include "kero/log/log_builder.h"
#include "kero/service/config_service.h"
#include "kero/service/io_event_loop_service.h"

using namespace kero;

kero::TcpServerService::TcpServerService() noexcept
    : Service{ServiceKind::kTcpServer,
              {ServiceKind::kConfig, ServiceKind::kIoEventLoop}} {}

auto
kero::TcpServerService::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  const auto config = agent.GetServiceAs<ConfigService>(ServiceKind::kConfig);
  if (!config) {
    return ResultT::Err(Error::From(
        Dict{}.Set("message", std::string{"ConfigService not found"}).Take()));
  }

  const auto io_event_loop =
      agent.GetServiceAs<IoEventLoopService>(ServiceKind::kIoEventLoop);
  if (!io_event_loop) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message", std::string{"IoEventLoopService not found"})
            .Take()));
  }

  if (!agent.SubscribeEvent(EventSocketRead::kEvent, GetKind())) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message",
                 std::string{"Failed to subscribe to socket read event"})
            .Take()));
  }

  const auto port = config.Unwrap().GetConfig().GetOrDefault<double>("port", 0);
  if (port == 0) {
    return ResultT::Err(Error::From(
        Dict{}.Set("message", std::string{"port not found in config"}).Take()));
  }

  auto server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (!Fd::IsValid(server_fd)) {
    return ResultT::Err(Error::From(
        Errno::FromErrno()
            .IntoDict()
            .Set("message", std::string{"Failed to create server socket"})
            .Take()));
  }

  int reuse{1};
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    return ResultT::Err(Error::From(
        Errno::FromErrno()
            .IntoDict()
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
            .IntoDict()
            .Set("message", std::string{"Failed to bind server socket"})
            .Set("port", static_cast<double>(port))
            .Take()));
  }

  if (listen(server_fd, SOMAXCONN) < 0) {
    return ResultT::Err(Error::From(
        Errno::FromErrno()
            .IntoDict()
            .Set("message", std::string{"Failed to listen on server"})
            .Take()));
  }

  if (auto res = io_event_loop.Unwrap().AddFd(server_fd, {.in = true});
      res.IsErr()) {
    return ResultT::Err(Error::From(res.TakeErr()));
  }

  server_fd_ = server_fd;
  return ResultT::Ok(Void{});
}

auto
kero::TcpServerService::OnDestroy(Agent& agent) noexcept -> void {
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
kero::TcpServerService::OnEvent(Agent& agent,
                                const std::string& event,
                                const Dict& data) noexcept -> void {
  if (event == EventSocketRead::kEvent) {
    const auto fd = data.GetOrDefault<double>(EventSocketRead::kFd, -1);
    if (fd == server_fd_) {
      auto io_event_loop =
          agent.GetServiceAs<IoEventLoopService>(ServiceKind::kIoEventLoop);
      if (!io_event_loop) {
        log::Error("Failed to get IoEventLoopService").Log();
        return;
      }

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

      agent.Invoke(
          EventSocketOpen::kEvent,
          Dict{}
              .Set(EventSocketOpen::kFd, static_cast<double>(client_fd))
              .Take());
    }
  }
}
