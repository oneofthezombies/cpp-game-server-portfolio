#include "main_event_loop_handler_linux.h"

#include <cassert>

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "core/tiny_json.h"
#include "core/utils_linux.h"

#include "config.h"
#include "event_loop.h"

using namespace engine;

auto engine::MainEventLoopHandlerLinux::OnInit(
    const Config &config, const EventLoop &event_loop) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  auto server_fd_raw = socket(AF_INET, SOCK_STREAM, 0);
  if (!FileDescriptorLinux::IsValid(server_fd_raw)) {
    return ResultT{Error{Symbol::kMainEventLoopHandlerLinuxServerSocketFailed,
                         core::TinyJson{}
                             .Set("linux_error", core::LinuxError::FromErrno())
                             .ToString()}};
  }

  auto server_fd = FileDescriptorLinux{server_fd_raw};
  if (auto result = server_fd.UpdateNonBlocking(); result.IsErr()) {
    return ResultT{Error{std::move(result.Err())}};
  }

  struct sockaddr_in server_addr {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(config.port);

  if (bind(server_fd.AsRaw(), (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    return ResultT{
        Error{Symbol::kMainEventLoopHandlerLinuxServerSocketBindFailed,
              core::TinyJson{}
                  .Set("linux_error", core::LinuxError::FromErrno())
                  .ToString()}};
  }

  if (listen(server_fd.AsRaw(), SOMAXCONN) < 0) {
    return ResultT{
        Error{Symbol::kMainEventLoopHandlerLinuxServerSocketListenFailed,
              core::TinyJson{}
                  .Set("linux_error", core::LinuxError::FromErrno())
                  .ToString()}};
  }

  if (auto res = event_loop.Add(server_fd.AsRaw(), EPOLLIN); res.IsErr()) {
    return res;
  }

  server_fd_.reset(new FileDescriptorLinux{std::move(server_fd)});
  return ResultT{Void{}};
}

auto engine::MainEventLoopHandlerLinux::OnSessionEvent(
    const SessionId session_id, const uint32_t events) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  assert(server_fd_ != nullptr && "server_fd must not be nullptr");

  const bool is_epoll_in = events & EPOLLIN;
  if (!is_epoll_in) {
    return ResultT{
        Error{Symbol::kMainEventLoopHandlerLinuxUnexpectedSessionEvent,
              core::TinyJson{}
                  .Set("session_id", session_id)
                  .Set("events", events)
                  .ToString()}};
  }

  const auto fd_res = FileDescriptorLinux::ParseSessionIdToFd(session_id);
  if (fd_res.IsErr()) {
    return ResultT{std::move(fd_res.Err())};
  }

  const auto fd = fd_res.Ok();
  if (fd != server_fd_->AsRaw()) {
    return ResultT{Error{Symbol::kMainEventLoopHandlerLinuxUnexpectedSessionId,
                         core::TinyJson{}
                             .Set("session_id", session_id)
                             .Set("fd", fd)
                             .ToString()}};
  }

  struct sockaddr_in client_addr {};
  socklen_t addrlen = sizeof(struct sockaddr_in);
  auto client_fd =
      accept(server_fd_->AsRaw(), (struct sockaddr *)&client_addr, &addrlen);
  if (!FileDescriptorLinux::IsValid(client_fd)) {
    return ResultT{
        Error{Symbol::kMainEventLoopHandlerLinuxServerSocketAcceptFailed,
              core::TinyJson{}
                  .Set("linux_error", core::LinuxError::FromErrno())
                  .ToString()}};
  }

  if (auto res = FileDescriptorLinux::UpdateNonBlocking(client_fd);
      res.IsErr()) {
    return ResultT{std::move(res.Err())};
  }

  // TODO: send client_fd to primary event loop
  context_->mail_box.tx.Send(
      Mail{"main", "lobby", {{"client_fd", std::to_string(client_fd_raw)}}});

  return ResultT{Void{}};
}
