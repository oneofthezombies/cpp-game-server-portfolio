#ifndef SERVER_LOBBY_EVENT_LOOP_LINUX_H
#define SERVER_LOBBY_EVENT_LOOP_LINUX_H

#include "core/core.h"

#include "event_loop_linux.h"

class LinuxLobbyEventLoop final : private core::NonCopyable, core::Movable {
public:
  [[nodiscard]] auto Run() noexcept -> Result<core::Void>;

private:
  explicit LinuxLobbyEventLoop(LinuxEventLoop &&event_loop) noexcept;

  [[nodiscard]] auto OnEventLoopEvent(const LinuxEventLoopEvent &event) noexcept
      -> Result<core::Void>;
  [[nodiscard]] auto OnEpollEvent(const struct epoll_event &event) noexcept
      -> Result<core::Void>;

  LinuxEventLoop event_loop_;

  friend class LinuxLobbyEventLoopBuilder;
};

class LinuxLobbyEventLoopBuilder final : private core::NonCopyable,
                                         private core::NonMovable {
public:
  [[nodiscard]] auto
  Build(core::Rx<LinuxEventLoopEvent> &&main_to_lobby_rx) const noexcept
      -> Result<LinuxLobbyEventLoop>;
};

#endif // SERVER_LOBBY_EVENT_LOOP_LINUX_H
