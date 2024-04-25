#ifndef SERVER_LOBBY_EVENT_LOOP_LINUX_H
#define SERVER_LOBBY_EVENT_LOOP_LINUX_H

#include <unordered_set>

#include "core/core.h"

#include "event_loop_linux.h"

class LobbyEventLoopLinux final : private core::NonCopyable, core::Movable {
public:
  [[nodiscard]] auto Run() noexcept -> Result<core::Void>;

private:
  explicit LobbyEventLoopLinux(
      EventLoopLinux &&event_loop,
      core::Tx<EventLoopLinuxEvent> &&lobby_to_battle_tx) noexcept;

  [[nodiscard]] auto OnEventLoopEvent(const EventLoopLinuxEvent &event) noexcept
      -> Result<core::Void>;
  [[nodiscard]] auto OnEpollEvent(const struct epoll_event &event) noexcept
      -> Result<core::Void>;
  [[nodiscard]] auto
  OnClientFdInserted(const FileDescriptorLinux::Raw client_fd_raw) noexcept
      -> Result<core::Void>;

  [[nodiscard]] auto
  OnClientFdMatched(const FileDescriptorLinux::Raw client_fd_0,
                    const FileDescriptorLinux::Raw client_fd_1) noexcept
      -> Result<core::Void>;

  [[nodiscard]] auto
  AddClientFd(const FileDescriptorLinux::Raw client_fd) noexcept
      -> Result<core::Void>;
  [[nodiscard]] auto
  DeleteClientFd(const FileDescriptorLinux::Raw client_fd) noexcept
      -> Result<core::Void>;

  EventLoopLinux event_loop_;
  core::Tx<EventLoopLinuxEvent> lobby_to_battle_tx_;
  std::unordered_set<FileDescriptorLinux::Raw> client_fds_;

  friend class LobbyEventLoopLinuxBuilder;
};

class LobbyEventLoopLinuxBuilder final : private core::NonCopyable,
                                         private core::NonMovable {
public:
  [[nodiscard]] auto
  Build(core::Rx<EventLoopLinuxEvent> &&main_to_lobby_rx,
        core::Rx<EventLoopLinuxEvent> &&battle_to_lobby_rx,
        core::Tx<EventLoopLinuxEvent> &&lobby_to_battle_tx) const noexcept
      -> Result<LobbyEventLoopLinux>;
};

#endif // SERVER_LOBBY_EVENT_LOOP_LINUX_H
