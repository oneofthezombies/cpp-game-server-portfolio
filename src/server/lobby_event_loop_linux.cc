#include "lobby_event_loop_linux.h"
#include "server/event_loop_linux.h"
#include "server/file_descriptor_linux.h"

#include <sys/epoll.h>

LinuxLobbyEventLoop::LinuxLobbyEventLoop(LinuxEventLoop &&event_loop) noexcept
    : event_loop_{std::move(event_loop)} {}

auto LinuxLobbyEventLoop::Run() noexcept -> Result<core::Void> {
  return event_loop_.Run(
      [this](const LinuxEventLoopEvent &event) {
        return OnEventLoopEvent(event);
      },
      [this](const struct epoll_event &event) { return OnEpollEvent(event); });
}

auto LinuxLobbyEventLoop::OnEventLoopEvent(
    const LinuxEventLoopEvent &event) noexcept -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  if (auto found = event.find("client_fd"); found != event.end()) {
    const auto &client_fd_str = found->second;
    auto client_fd_res =
        core::ParseNumberString<LinuxFileDescriptor::Raw>(client_fd_str);
    if (client_fd_res.IsErr()) {
      return ResultT{Error{
          Symbol::kLinuxLobbyEventLoopClientFdConversionFailed,
          SB{}.Add("client_fd_str", client_fd_str)
              .Add("errc", std::make_error_code(client_fd_res.Err()).message())
              .Build()}};
    }

    const auto client_fd_raw = client_fd_res.Ok();
    if (auto res = event_loop_.Add(client_fd_raw, EPOLLIN | EPOLLET);
        res.IsErr()) {
      return res;
    }

    // TODO: Add session to the map and check matching
  }

  return ResultT{core::Void{}};
}

auto LinuxLobbyEventLoop::OnEpollEvent(const struct epoll_event &event) noexcept
    -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  return ResultT{core::Void{}};
}

auto LinuxLobbyEventLoopBuilder::Build(
    core::Rx<LinuxEventLoopEvent> &&main_to_lobby_rx) const noexcept
    -> Result<LinuxLobbyEventLoop> {
  using ResultT = Result<LinuxLobbyEventLoop>;

  auto event_loop_res =
      LinuxEventLoopBuilder{}.Build(std::move(main_to_lobby_rx));
  if (event_loop_res.IsErr()) {
    return ResultT{std::move(event_loop_res.Err())};
  }

  return ResultT{LinuxLobbyEventLoop{std::move(event_loop_res.Ok())}};
}
