#include "main_event_loop_linux.h"

#include <cassert>

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "core/core.h"
#include "core/spsc_channel.h"
#include "core/tiny_json.h"

#include "event_loop_linux.h"
#include "file_descriptor_linux.h"
#include "utils_linux.h"

MainEventLoopLinux::MainEventLoopLinux(FileDescriptorLinux &&server_fd) noexcept
    : EventLoopLinux{}, server_fd_{std::move(server_fd)} {
  assert(server_fd_.IsValid() && "server fd must be valid");
}

auto MainEventLoopLinux::Init(const std::string_view name) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Super::Init(name); res.IsErr()) {
    return res;
  }

  if (auto res = Add(server_fd_.AsRaw(), EPOLLIN); res.IsErr()) {
    return res;
  }

  return ResultT{Void{}};
}

auto MainEventLoopLinux::OnMailReceived(const Mail &mail) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  return ResultT{Void{}};
}

auto MainEventLoopLinux::OnEpollEventReceived(
    const struct epoll_event &event) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (event.data.fd != server_fd_.AsRaw()) {
    return ResultT{Error{Symbol::kMainEventLoopLinuxUnexpectedFd,
                         TinyJson{}.Set("fd", event.data.fd).ToString()}};
  }

  struct sockaddr_in client_addr {};
  socklen_t addrlen = sizeof(struct sockaddr_in);
  auto client_fd_raw =
      accept(server_fd_.AsRaw(), (struct sockaddr *)&client_addr, &addrlen);
  if (!FileDescriptorLinux::IsValid(client_fd_raw)) {
    return ResultT{Error{
        Symbol::kMainEventLoopLinuxServerSocketAcceptFailed,
        TinyJson{}.Set("linux_error", LinuxError::FromErrno()).ToString()}};
  }

  if (auto res = FileDescriptorLinux::UpdateNonBlocking(client_fd_raw);
      res.IsErr()) {
    return ResultT{std::move(res.Err())};
  }

  context_->mail_box.tx.Send(
      Mail{"main", "lobby", {{"client_fd", std::to_string(client_fd_raw)}}});
  return ResultT{Void{}};
}

auto MainEventLoopLinux::Builder::Build(const uint16_t port) const noexcept
    -> Result<MainEventLoopLinux> {
  using ResultT = Result<MainEventLoopLinux>;

  auto server_fd_raw = socket(AF_INET, SOCK_STREAM, 0);
  if (!FileDescriptorLinux::IsValid(server_fd_raw)) {
    return ResultT{Error{
        Symbol::kMainEventLoopLinuxServerSocketFailed,
        TinyJson{}.Set("linux_error", LinuxError::FromErrno()).ToString()}};
  }

  auto server_fd = FileDescriptorLinux{server_fd_raw};
  if (auto result = server_fd.UpdateNonBlocking(); result.IsErr()) {
    return ResultT{Error{std::move(result.Err())}};
  }

  struct sockaddr_in server_addr {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_fd.AsRaw(), (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    return ResultT{Error{
        Symbol::kMainEventLoopLinuxServerSocketBindFailed,
        TinyJson{}.Set("linux_error", LinuxError::FromErrno()).ToString()}};
  }

  if (listen(server_fd.AsRaw(), SOMAXCONN) < 0) {
    return ResultT{Error{
        Symbol::kMainEventLoopLinuxServerSocketListenFailed,
        TinyJson{}.Set("linux_error", LinuxError::FromErrno()).ToString()}};
  }

  return ResultT{MainEventLoopLinux{std::move(server_fd)}};
}
