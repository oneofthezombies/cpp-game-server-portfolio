#include "tcp_server_service.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include "kero/engine/agent.h"
#include "kero/engine/config_service.h"
#include "kero/engine/constants.h"
#include "kero/engine/io_event_loop_service.h"

using namespace kero;

kero::TcpServerService::TcpServerService() noexcept
    : Service{ServiceKind::kTcpServer} {}

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

  if (!agent.SubscribeEvent(EventSocketError::kEvent, GetKind())) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message",
                 std::string{"Failed to subscribe to socket error event"})
            .Take()));
  }

  if (!agent.SubscribeEvent(EventSocketClose::kEvent, GetKind())) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message",
                 std::string{"Failed to subscribe to socket close event"})
            .Take()));
  }

  if (!agent.SubscribeEvent(EventSocketRead::kEvent, GetKind())) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message",
                 std::string{"Failed to subscribe to socket read event"})
            .Take()));
  }

  const auto port = config.Unwrap().GetConfig().GetOrDefault("port", 0);
  if (port == 0) {
    return ResultT::Err(Error::From(
        Dict{}.Set("message", std::string{"Port not found in config"}).Take()));
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
            .Set("port", static_cast<uint16_t>(port))
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

  return ResultT::Ok(Void{});
}

auto
kero::TcpServerService::OnDestroy(Agent& agent) noexcept -> void {
  if (!Fd::IsValid(server_fd_)) {
    return;
  }

  if (auto res = Fd::Close(server_fd_); res.IsErr()) {
    // TODO: log error
  }
}

auto
kero::TcpServerService::OnEvent(Agent& agent,
                                const std::string& event,
                                const Dict& data) noexcept -> void {
  if (event == EventSocketError::kEvent) {
    const auto fd = data.GetOrDefault(EventSocketError::kFd, -1);
    if (fd == server_fd_) {
      const auto error_code =
          data.GetOrDefault(EventSocketError::kErrorCode, 0);
      const auto error_description =
          data.GetOrDefault(EventSocketError::kErrorDescription, std::string{});
      // TODO: log error
    }
  } else if (event == EventSocketClose::kEvent) {
    const auto fd = data.GetOrDefault(EventSocketClose::kFd, -1);
    if (fd == server_fd_) {
      // TODO: log error
    }
  } else if (event == EventSocketRead::kEvent) {
    const auto fd = data.GetOrDefault(EventSocketRead::kFd, -1);
    if (fd == server_fd_) {
      auto io_event_loop =
          agent.GetServiceAs<IoEventLoopService>(ServiceKind::kIoEventLoop);
      if (!io_event_loop) {
        // TODO: log error
        return;
      }

      struct sockaddr_in client_addr {};
      socklen_t addrlen = sizeof(struct sockaddr_in);
      auto client_fd =
          accept(server_fd_, (struct sockaddr*)&client_addr, &addrlen);
      if (!Fd::IsValid(client_fd)) {
        // TODO: log error
        return;
      }

      if (auto res = Fd::UpdateNonBlocking(client_fd); res.IsErr()) {
        // TODO: log error
        return;
      }

      // send mail or invoke event
    }
  }
}
