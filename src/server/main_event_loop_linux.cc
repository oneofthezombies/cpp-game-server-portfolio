#include "main_event_loop_linux.h"

#include <cassert>

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "core.h"
#include "core/spsc_channel.h"

#include "event_loop_linux.h"
#include "server/file_descriptor_linux.h"
#include "utils_linux.h"

LinuxMainEventLoop::LinuxMainEventLoop(
    core::Tx<LinuxEventLoopEvent> &&main_to_lobby_tx,
    LinuxEventLoop &&event_loop, LinuxFileDescriptor &&server_fd) noexcept
    : main_to_lobby_tx_{std::move(main_to_lobby_tx)},
      event_loop_{std::move(event_loop)}, server_fd_{std::move(server_fd)} {
  assert(server_fd_.IsValid() && "server fd must be valid");
}

auto LinuxMainEventLoop::Run() noexcept -> Result<core::Void> {
  if (auto res = event_loop_.Add(server_fd_.AsRaw(), EPOLLIN); res.IsErr()) {
    return res;
  }

  return event_loop_.Run(
      [this](const LinuxEventLoopEvent &event) {
        return OnEventLoopEvent(event);
      },
      [this](const struct epoll_event &event) { return OnEpollEvent(event); });
}

auto LinuxMainEventLoop::OnEventLoopEvent(
    const LinuxEventLoopEvent &event) noexcept -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  if (auto found = event.find("shutdown"); found != event.end()) {
    main_to_lobby_tx_.Send({{"shutdown", ""}});
  }

  return ResultT{core::Void{}};
}

auto LinuxMainEventLoop::OnEpollEvent(const struct epoll_event &event) noexcept
    -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  if (event.data.fd != server_fd_.AsRaw()) {
    return ResultT{Error{Symbol::kLinuxMainEventLoopUnexpectedFd,
                         SB{}.Add("fd", event.data.fd).Build()}};
  }

  struct sockaddr_in client_addr {};
  socklen_t addrlen = sizeof(struct sockaddr_in);
  auto client_fd_raw =
      accept(server_fd_.AsRaw(), (struct sockaddr *)&client_addr, &addrlen);
  if (!LinuxFileDescriptor::IsValid(client_fd_raw)) {
    return ResultT{Error{Symbol::kLinuxMainEventLoopServerSocketAcceptFailed,
                         SB{}.Add(core::LinuxError::FromErrno()).Build()}};
  }

  if (auto res = LinuxFileDescriptor::UpdateNonBlocking(client_fd_raw);
      res.IsErr()) {
    return ResultT{std::move(res.Err())};
  }

  main_to_lobby_tx_.Send({{"client_fd", std::to_string(client_fd_raw)}});
  return ResultT{core::Void{}};
}

auto LinuxMainEventLoopBuilder::Build(
    const uint16_t port, core::Rx<LinuxEventLoopEvent> &&signal_to_main_rx,
    core::Tx<LinuxEventLoopEvent> &&main_to_lobby_tx) const noexcept
    -> Result<LinuxMainEventLoop> {
  using ResultT = Result<LinuxMainEventLoop>;

  auto event_loop_res =
      LinuxEventLoopBuilder{}.Build(std::move(signal_to_main_rx));
  if (event_loop_res.IsErr()) {
    return ResultT{std::move(event_loop_res.Err())};
  }

  auto server_fd_raw = socket(AF_INET, SOCK_STREAM, 0);
  if (!LinuxFileDescriptor::IsValid(server_fd_raw)) {
    return ResultT{Error{Symbol::kLinuxMainEventLoopServerSocketFailed,
                         SB{}.Add(core::LinuxError::FromErrno()).Build()}};
  }

  auto server_fd = LinuxFileDescriptor{server_fd_raw};
  if (auto result = server_fd.UpdateNonBlocking(); result.IsErr()) {
    return ResultT{Error{std::move(result.Err())}};
  }

  struct sockaddr_in server_addr {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_fd.AsRaw(), (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    return ResultT{Error{Symbol::kLinuxMainEventLoopServerSocketBindFailed,
                         SB{}.Add(core::LinuxError::FromErrno()).Build()}};
  }

  if (listen(server_fd.AsRaw(), SOMAXCONN) < 0) {
    return ResultT{Error{Symbol::kLinuxMainEventLoopServerSocketListenFailed,
                         SB{}.Add(core::LinuxError::FromErrno()).Build()}};
  }

  return ResultT{LinuxMainEventLoop{std::move(main_to_lobby_tx),
                                    std::move(event_loop_res.Ok()),
                                    std::move(server_fd)}};
}
