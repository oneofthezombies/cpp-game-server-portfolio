#include "main_event_loop_handler_linux.h"

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <cassert>

#include "config.h"
#include "core/tiny_json.h"
#include "core/utils_linux.h"
#include "event_loop.h"

using namespace engine;

engine::MainEventLoopHandlerLinux::MainEventLoopHandlerLinux(
    std::string &&primary_event_loop_name) noexcept
    : EventLoopHandler{},
      primary_event_loop_name_{std::move(primary_event_loop_name)} {}

auto
engine::MainEventLoopHandlerLinux::OnInit(EventLoop &event_loop,
                                          const Config &config) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  auto server_fd_raw = socket(AF_INET, SOCK_STREAM, 0);
  if (!FileDescriptorLinux::IsValid(server_fd_raw)) {
    return ResultT{
        Error::From(kMainEventLoopHandlerLinuxServerSocketFailed,
                    core::JsonParser{}
                        .Set("linux_error", core::LinuxError::FromErrno())
                        .IntoMap())};
  }

  int reuse{1};
  if (setsockopt(server_fd_raw,
                 SOL_SOCKET,
                 SO_REUSEADDR,
                 &reuse,
                 sizeof(reuse)) < 0) {
    return ResultT{
        Error::From(kMainEventLoopHandlerLinuxServerSocketSetOptFailed,
                    core::JsonParser{}
                        .Set("linux_error", core::LinuxError::FromErrno())
                        .IntoMap())};
  }

  auto server_fd = FileDescriptorLinux{server_fd_raw};
  if (auto result = server_fd.UpdateNonBlocking(); result.IsErr()) {
    return ResultT{Error{std::move(result.Err())}};
  }

  struct sockaddr_in server_addr {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(config.port);

  if (bind(server_fd.AsRaw(),
           (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    return ResultT{
        Error::From(kMainEventLoopHandlerLinuxServerSocketBindFailed,
                    core::JsonParser{}
                        .Set("linux_error", core::LinuxError::FromErrno())
                        .IntoMap())};
  }

  if (listen(server_fd.AsRaw(), SOMAXCONN) < 0) {
    return ResultT{
        Error::From(kMainEventLoopHandlerLinuxServerSocketListenFailed,
                    core::JsonParser{}
                        .Set("linux_error", core::LinuxError::FromErrno())
                        .IntoMap())};
  }

  if (auto res = event_loop.Add(server_fd.AsRaw(), {.in = true}); res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  server_fd_.reset(new FileDescriptorLinux{std::move(server_fd)});
  return ResultT{Void{}};
}

auto
engine::MainEventLoopHandlerLinux::OnMail(EventLoop &event_loop,
                                          const Mail &mail) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  // noop

  return ResultT{Void{}};
}

auto
engine::MainEventLoopHandlerLinux::OnSocketIn(EventLoop &event_loop,
                                              const SocketId socket_id) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  assert(server_fd_ != nullptr && "server_fd must not be nullptr");

  auto server_fd_res = FileDescriptorLinux::ParseSocketIdToFd(socket_id);
  if (server_fd_res.IsErr()) {
    return ResultT{Error::From(server_fd_res.TakeErr())};
  }

  const auto server_fd = server_fd_res.Ok();
  if (server_fd != server_fd_->AsRaw()) {
    return ResultT{Error::From(kMainEventLoopHandlerLinuxUnexpectedSocketId,
                               core::JsonParser{}
                                   .Set("socket_id", socket_id)
                                   .Set("server_fd", server_fd)
                                   .IntoMap())};
  }

  struct sockaddr_in client_addr {};
  socklen_t addrlen = sizeof(struct sockaddr_in);
  auto client_fd =
      accept(server_fd_->AsRaw(), (struct sockaddr *)&client_addr, &addrlen);
  if (!FileDescriptorLinux::IsValid(client_fd)) {
    return ResultT{
        Error::From(kMainEventLoopHandlerLinuxServerSocketAcceptFailed,
                    core::JsonParser{}
                        .Set("linux_error", core::LinuxError::FromErrno())
                        .IntoMap())};
  }

  if (auto res = FileDescriptorLinux::UpdateNonBlocking(client_fd);
      res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  auto client_socket_id_res = FileDescriptorLinux::ParseFdToSocketId(client_fd);
  if (client_socket_id_res.IsErr()) {
    return ResultT{Error::From(client_socket_id_res.TakeErr())};
  }

  const auto client_socket_id = client_socket_id_res.Ok();
  event_loop.SendMail(std::string{primary_event_loop_name_},
                      std::move(core::JsonParser{}
                                    .Set("kind", "connect")
                                    .Set("socket_id", client_socket_id)));
  return ResultT{Void{}};
}

auto
engine::MainEventLoopHandlerLinux::OnSocketHangUp(
    EventLoop &event_loop, const SocketId socket_id) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  assert(server_fd_ != nullptr && "server_fd must not be nullptr");

  auto fd_res = FileDescriptorLinux::ParseSocketIdToFd(socket_id);
  if (fd_res.IsErr()) {
    return ResultT{Error::From(fd_res.TakeErr())};
  }

  const auto fd = fd_res.Ok();
  core::JsonParser{}
      .Set("message", "socket hang up")
      .Set("name", event_loop.GetName())
      .Set("socket_id", socket_id)
      .Set("fd", fd)
      .LogLn();
  return ResultT{Void{}};
}

auto
engine::MainEventLoopHandlerLinux::OnSocketError(
    EventLoop &event_loop,
    const SocketId socket_id,
    const int code,
    const std::string_view description) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  assert(server_fd_ != nullptr && "server_fd must not be nullptr");

  auto fd_res = FileDescriptorLinux::ParseSocketIdToFd(socket_id);
  if (fd_res.IsErr()) {
    return ResultT{Error::From(fd_res.TakeErr())};
  }

  const auto fd = fd_res.Ok();
  core::JsonParser{}
      .Set("message", "socket error")
      .Set("name", event_loop.GetName())
      .Set("socket_id", socket_id)
      .Set("fd", fd)
      .Set("code", code)
      .Set("description", description)
      .LogLn();
  return ResultT{Void{}};
}
