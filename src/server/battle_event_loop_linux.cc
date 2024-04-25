#include "battle_event_loop_linux.h"

#include <iostream>

#include <sys/epoll.h>

#include "event_loop_linux.h"

BattleEventLoopLinux::BattleEventLoopLinux(
    EventLoopLinux &&event_loop,
    Tx<EventLoopLinuxEvent> &&battle_to_lobby_tx) noexcept
    : event_loop_{std::move(event_loop)},
      battle_to_lobby_tx_{std::move(battle_to_lobby_tx)} {
  event_handlers_.emplace(
      "matched_client_fds",
      [this](const std::string &value) { return OnMatchedClientFds(value); });
}

auto BattleEventLoopLinux::Run() noexcept -> Result<Void> {
  return event_loop_.Run(
      [this](const EventLoopLinuxEvent &event) {
        return OnEventLoopEvent(event);
      },
      [this](const struct epoll_event &event) { return OnEpollEvent(event); });
}

auto BattleEventLoopLinux::OnEventLoopEvent(
    const EventLoopLinuxEvent &event) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  for (const auto &[key, value] : event) {
    std::cout << "battlt event: " << key << " " << value << std::endl;

    const auto found = event_handlers_.find(key);
    if (found == event_handlers_.end()) {
      return ResultT{Error{Symbol::kBattleEventLoopLinuxHandlerNotFound,
                           SB{}.Add("key", key).Add("value", value).Build()}};
    }

    if (auto res = found->second(value); res.IsErr()) {
      return res;
    }
  }

  return ResultT{Void{}};
}

auto BattleEventLoopLinux::OnEpollEvent(
    const struct epoll_event &event) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  return ResultT{Void{}};
}

auto BattleEventLoopLinux::AddClientFd(const FileDescriptorLinux::Raw client_fd,
                                       const RoomId room_id) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = event_loop_.Add(client_fd, EPOLLIN | EPOLLET); res.IsErr()) {
    return res;
  }

  if (client_fds_.find(client_fd) != client_fds_.end()) {
    const Error error{
        Symbol::kBattleEventLoopLinuxClientFdAlreadyExists,
        SB{}.Add("Delete old client fd").Add("client_fd", client_fd).Build()};
    std::cout << error << std::endl;
    client_fds_.erase(client_fd);
  }

  client_fds_.emplace(client_fd, room_id);
  return ResultT{Void{}};
}

auto BattleEventLoopLinux::DeleteClientFd(
    const FileDescriptorLinux::Raw client_fd) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  client_fds_.erase(client_fd);
  if (auto res = event_loop_.Delete(client_fd); res.IsErr()) {
    return res;
  }

  return ResultT{Void{}};
}

auto BattleEventLoopLinux::NextRoomId() noexcept -> RoomId {
  return next_room_id_++;
}

auto BattleEventLoopLinux::OnMatchedClientFds(
    const std::string &matched_client_fds) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  const auto matched_client_fds_str = std::string_view{matched_client_fds};
  const auto comma = matched_client_fds_str.find(",");
  if (comma == std::string::npos) {
    return ResultT{
        Error{Symbol::kBattleEventLoopLinuxMatchedClientFdsNoComma,
              SB{}.Add("matched_client_fds", matched_client_fds_str).Build()}};
  }

  const auto client_fd_0_str = matched_client_fds_str.substr(0, comma);
  const auto client_fd_1_str = matched_client_fds_str.substr(comma + 1);

  auto client_fd_0_res =
      ParseNumberString<FileDescriptorLinux::Raw>(client_fd_0_str);
  if (client_fd_0_res.IsErr()) {
    return ResultT{Error{
        Symbol::kBattleEventLoopLinuxClientFdConversionFailed,
        SB{}.Add("client_fd_0", client_fd_0_str)
            .Add("errc", std::make_error_code(client_fd_0_res.Err()).message())
            .Build()}};
  }

  auto client_fd_1_res =
      ParseNumberString<FileDescriptorLinux::Raw>(client_fd_1_str);
  if (client_fd_1_res.IsErr()) {
    return ResultT{Error{
        Symbol::kBattleEventLoopLinuxClientFdConversionFailed,
        SB{}.Add("client_fd_1", client_fd_1_str)
            .Add("errc", std::make_error_code(client_fd_1_res.Err()).message())
            .Build()}};
  }

  const auto room_id = NextRoomId();
  const auto client_fd_0 = client_fd_0_res.Ok();
  if (auto res = AddClientFd(client_fd_0, room_id); res.IsErr()) {
    return res;
  }

  Defer defer{[this, client_fd_0] {
    if (auto res = DeleteClientFd(client_fd_0); res.IsErr()) {
      std::cout << res.Err() << std::endl;
    }
  }};

  if (auto res = AddClientFd(client_fd_1_res.Ok(), room_id); res.IsErr()) {
    return res;
  }

  defer.Cancel();
  return ResultT{Void{}};
}

auto BattleEventLoopLinuxBuilder::Build(
    Rx<EventLoopLinuxEvent> &&lobby_to_battle_rx,
    Tx<EventLoopLinuxEvent> &&battle_to_lobby_tx) const noexcept
    -> Result<BattleEventLoopLinux> {
  using ResultT = Result<BattleEventLoopLinux>;

  std::vector<Rx<EventLoopLinuxEvent>> event_loop_rxs;
  event_loop_rxs.emplace_back(std::move(lobby_to_battle_rx));
  auto event_loop_res =
      EventLoopLinuxBuilder{}.Build(std::move(event_loop_rxs));
  if (event_loop_res.IsErr()) {
    return ResultT{std::move(event_loop_res.Err())};
  }

  return ResultT{BattleEventLoopLinux{std::move(event_loop_res.Ok()),
                                      std::move(battle_to_lobby_tx)}};
}
