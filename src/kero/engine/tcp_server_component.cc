#include "tcp_server_component.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include "kero/engine/agent.h"
#include "kero/engine/config_component.h"
#include "kero/engine/constants.h"
#include "kero/engine/io_event_loop_component.h"

using namespace kero;

kero::TcpServerComponent::TcpServerComponent() noexcept
    : Component{ComponentKind::kTcpServer} {}

auto
kero::TcpServerComponent::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  const auto config =
      agent.GetComponentAs<ConfigComponent>(ComponentKind::kConfig);
  if (!config) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message", std::string{"ConfigComponent not found"})
            .Take()));
  }

  const auto io_event_loop =
      agent.GetComponentAs<IoEventLoopComponent>(ComponentKind::kIoEventLoop);
  if (!io_event_loop) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message", std::string{"IoEventLoopComponent not found"})
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
kero::TcpServerComponent::OnDestroy(Agent& agent) noexcept -> void {
  if (!Fd::IsValid(server_fd_)) {
    return;
  }

  if (auto res = Fd::Close(server_fd_); res.IsErr()) {
    // TODO: log error
  }
}
