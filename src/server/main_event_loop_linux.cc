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

MainEventLoopLinux::MainEventLoopLinux(
    core::Tx<EventLoopLinuxEvent> &&main_to_lobby_tx,
    EventLoopLinux &&event_loop, FileDescriptorLinux &&server_fd) noexcept
    : main_to_lobby_tx_{std::move(main_to_lobby_tx)},
      event_loop_{std::move(event_loop)}, server_fd_{std::move(server_fd)} {
  assert(server_fd_.IsValid() && "server fd must be valid");
}

auto MainEventLoopLinux::Run() noexcept -> Result<core::Void> {
  if (auto res = event_loop_.Add(server_fd_.AsRaw(), EPOLLIN); res.IsErr()) {
    return res;
  }

  return event_loop_.Run(
      [this](const EventLoopLinuxEvent &event) {
        return OnEventLoopEvent(event);
      },
      [this](const struct epoll_event &event) { return OnEpollEvent(event); });
}

auto MainEventLoopLinux::OnEventLoopEvent(
    const EventLoopLinuxEvent &event) noexcept -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  if (auto found = event.find("shutdown"); found != event.end()) {
    main_to_lobby_tx_.Send({{"shutdown", ""}});
  }

  return ResultT{core::Void{}};
}

auto MainEventLoopLinux::OnEpollEvent(const struct epoll_event &event) noexcept
    -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  if (event.data.fd != server_fd_.AsRaw()) {
    return ResultT{Error{Symbol::kMainEventLoopLinuxUnexpectedFd,
                         SB{}.Add("fd", event.data.fd).Build()}};
  }

  struct sockaddr_in client_addr {};
  socklen_t addrlen = sizeof(struct sockaddr_in);
  auto client_fd_raw =
      accept(server_fd_.AsRaw(), (struct sockaddr *)&client_addr, &addrlen);
  if (!FileDescriptorLinux::IsValid(client_fd_raw)) {
    return ResultT{Error{Symbol::kMainEventLoopLinuxServerSocketAcceptFailed,
                         SB{}.Add(core::LinuxError::FromErrno()).Build()}};
  }

  if (auto res = FileDescriptorLinux::UpdateNonBlocking(client_fd_raw);
      res.IsErr()) {
    return ResultT{std::move(res.Err())};
  }

  main_to_lobby_tx_.Send({{"client_fd", std::to_string(client_fd_raw)}});
  return ResultT{core::Void{}};
}

auto MainEventLoopLinuxBuilder::Build(
    const uint16_t port, core::Rx<EventLoopLinuxEvent> &&signal_to_main_rx,
    core::Tx<EventLoopLinuxEvent> &&main_to_lobby_tx) const noexcept
    -> Result<MainEventLoopLinux> {
  using ResultT = Result<MainEventLoopLinux>;

  std::vector<core::Rx<EventLoopLinuxEvent>> event_loop_rxs;
  event_loop_rxs.emplace_back(std::move(signal_to_main_rx));
  auto event_loop_res =
      EventLoopLinuxBuilder{}.Build(std::move(event_loop_rxs));
  if (event_loop_res.IsErr()) {
    return ResultT{std::move(event_loop_res.Err())};
  }

  auto server_fd_raw = socket(AF_INET, SOCK_STREAM, 0);
  if (!FileDescriptorLinux::IsValid(server_fd_raw)) {
    return ResultT{Error{Symbol::kMainEventLoopLinuxServerSocketFailed,
                         SB{}.Add(core::LinuxError::FromErrno()).Build()}};
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
    return ResultT{Error{Symbol::kMainEventLoopLinuxServerSocketBindFailed,
                         SB{}.Add(core::LinuxError::FromErrno()).Build()}};
  }

  if (listen(server_fd.AsRaw(), SOMAXCONN) < 0) {
    return ResultT{Error{Symbol::kMainEventLoopLinuxServerSocketListenFailed,
                         SB{}.Add(core::LinuxError::FromErrno()).Build()}};
  }

  return ResultT{MainEventLoopLinux{std::move(main_to_lobby_tx),
                                    std::move(event_loop_res.Ok()),
                                    std::move(server_fd)}};
}
