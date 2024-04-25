#include "lobby_event_loop_linux.h"

#include <iostream>

#include <sys/epoll.h>

#include "event_loop_linux.h"
#include "file_descriptor_linux.h"

LobbyEventLoopLinux::LobbyEventLoopLinux(
    EventLoopLinux &&event_loop,
    Tx<EventLoopLinuxEvent> &&lobby_to_battle_tx) noexcept
    : event_loop_{std::move(event_loop)},
      lobby_to_battle_tx_{std::move(lobby_to_battle_tx)} {}

auto LobbyEventLoopLinux::Run() noexcept -> Result<Void> {
  return event_loop_.Run(
      [this](const EventLoopLinuxEvent &event) {
        return OnEventLoopEvent(event);
      },
      [this](const struct epoll_event &event) { return OnEpollEvent(event); });
}

auto LobbyEventLoopLinux::OnEventLoopEvent(
    const EventLoopLinuxEvent &event) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto found = event.find("client_fd"); found != event.end()) {
    const auto &client_fd_str = found->second;
    auto client_fd_res =
        ParseNumberString<FileDescriptorLinux::Raw>(client_fd_str);
    if (client_fd_res.IsErr()) {
      return ResultT{Error{
          Symbol::kLobbyEventLoopLinuxClientFdConversionFailed,
          SB{}.Add("client_fd_str", client_fd_str)
              .Add("errc", std::make_error_code(client_fd_res.Err()).message())
              .Build()}};
    }

    const auto client_fd_raw = client_fd_res.Ok();
    if (auto res = AddClientFd(client_fd_raw); res.IsErr()) {
      return res;
    }

    return OnClientFdInserted(client_fd_raw);
  }

  return ResultT{Void{}};
}

auto LobbyEventLoopLinux::OnEpollEvent(const struct epoll_event &event) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  return ResultT{Void{}};
}

auto LobbyEventLoopLinux::OnClientFdInserted(
    const FileDescriptorLinux::Raw client_fd_raw) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (client_fds_.size() >= 2) {
    const auto client_fd_0 = *client_fds_.begin();
    const auto client_fd_1 = *std::next(client_fds_.begin());
    if (auto res = OnClientFdMatched(client_fd_0, client_fd_1); res.IsErr()) {
      return res;
    }
  }

  return ResultT{Void{}};
}

auto LobbyEventLoopLinux::OnClientFdMatched(
    const FileDescriptorLinux::Raw client_fd_0,
    const FileDescriptorLinux::Raw client_fd_1) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = DeleteClientFd(client_fd_0); res.IsErr()) {
    return res;
  }

  if (auto res = DeleteClientFd(client_fd_1); res.IsErr()) {
    return res;
  }

  std::cout << "Matched " << client_fd_0 << " and " << client_fd_1 << std::endl;
  lobby_to_battle_tx_.Send(EventLoopLinuxEvent{
      {{"matched_client_fds",
        SB{}.Add(client_fd_0).Add(",").Add(client_fd_1).Build()}}});

  return ResultT{Void{}};
}

auto LobbyEventLoopLinux::AddClientFd(
    const FileDescriptorLinux::Raw client_fd) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = event_loop_.Add(client_fd, EPOLLIN | EPOLLET); res.IsErr()) {
    return res;
  }

  if (client_fds_.find(client_fd) != client_fds_.end()) {
    const Error error{
        Symbol::kLobbyEventLoopLinuxClientFdAlreadyExists,
        SB{}.Add("Delete old client fd").Add("client_fd", client_fd).Build()};
    std::cout << error << std::endl;
    client_fds_.erase(client_fd);
  }

  client_fds_.insert(client_fd);
  return ResultT{Void{}};
}

auto LobbyEventLoopLinux::DeleteClientFd(
    const FileDescriptorLinux::Raw client_fd) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  client_fds_.erase(client_fd);
  if (auto res = event_loop_.Delete(client_fd); res.IsErr()) {
    return res;
  }

  return ResultT{Void{}};
}

auto LobbyEventLoopLinuxBuilder::Build(
    Rx<EventLoopLinuxEvent> &&main_to_lobby_rx,
    Rx<EventLoopLinuxEvent> &&battle_to_lobby_rx,
    Tx<EventLoopLinuxEvent> &&lobby_to_battle_tx) const noexcept
    -> Result<LobbyEventLoopLinux> {
  using ResultT = Result<LobbyEventLoopLinux>;

  std::vector<Rx<EventLoopLinuxEvent>> event_loop_rxs;
  event_loop_rxs.push_back(std::move(main_to_lobby_rx));
  event_loop_rxs.push_back(std::move(battle_to_lobby_rx));
  auto event_loop_res =
      EventLoopLinuxBuilder{}.Build(std::move(event_loop_rxs));
  if (event_loop_res.IsErr()) {
    return ResultT{std::move(event_loop_res.Err())};
  }

  return ResultT{LobbyEventLoopLinux{std::move(event_loop_res.Ok()),
                                     std::move(lobby_to_battle_tx)}};
}
