#ifndef SERVER_LOBBY_EVENT_LOOP_LINUX_H
#define SERVER_LOBBY_EVENT_LOOP_LINUX_H

#include <unordered_set>

#include "core/core.h"

#include "event_loop_linux.h"

class LobbyEventLoopLinux final : private NonCopyable, Movable {
public:
  [[nodiscard]] auto Run() noexcept -> Result<Void>;

private:
  explicit LobbyEventLoopLinux(
      EventLoopLinux &&event_loop,
      Tx<EventLoopLinuxEvent> &&lobby_to_battle_tx) noexcept;

  [[nodiscard]] auto OnEventLoopEvent(const EventLoopLinuxEvent &event) noexcept
      -> Result<Void>;
  [[nodiscard]] auto OnEpollEvent(const struct epoll_event &event) noexcept
      -> Result<Void>;
  [[nodiscard]] auto
  OnClientFdInserted(const FileDescriptorLinux::Raw client_fd_raw) noexcept
      -> Result<Void>;

  [[nodiscard]] auto
  OnClientFdMatched(const FileDescriptorLinux::Raw client_fd_0,
                    const FileDescriptorLinux::Raw client_fd_1) noexcept
      -> Result<Void>;

  [[nodiscard]] auto
  AddClientFd(const FileDescriptorLinux::Raw client_fd) noexcept
      -> Result<Void>;
  [[nodiscard]] auto
  DeleteClientFd(const FileDescriptorLinux::Raw client_fd) noexcept
      -> Result<Void>;

  EventLoopLinux event_loop_;
  Tx<EventLoopLinuxEvent> lobby_to_battle_tx_;
  std::unordered_set<FileDescriptorLinux::Raw> client_fds_;

  friend class LobbyEventLoopLinuxBuilder;
};

class LobbyEventLoopLinuxBuilder final : private NonCopyable,
                                         private NonMovable {
public:
  [[nodiscard]] auto
  Build(Rx<EventLoopLinuxEvent> &&main_to_lobby_rx,
        Rx<EventLoopLinuxEvent> &&battle_to_lobby_rx,
        Tx<EventLoopLinuxEvent> &&lobby_to_battle_tx) const noexcept
      -> Result<LobbyEventLoopLinux>;
};

#endif // SERVER_LOBBY_EVENT_LOOP_LINUX_H
