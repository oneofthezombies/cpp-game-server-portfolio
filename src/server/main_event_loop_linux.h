#ifndef SERVER_MAIN_EVENT_LOOP_LINUX_H
#define SERVER_MAIN_EVENT_LOOP_LINUX_H

#include "core/core.h"

#include "event_loop_linux.h"
#include "server/file_descriptor_linux.h"

class LinuxMainEventLoop final : private core::NonCopyable, core::Movable {
public:
  [[nodiscard]] auto Run() noexcept -> Result<core::Void>;

private:
  explicit LinuxMainEventLoop(core::Tx<LinuxEventLoopEvent> &&main_to_lobby_tx,
                              LinuxEventLoop &&event_loop,
                              LinuxFileDescriptor &&server_fd) noexcept;

  [[nodiscard]] auto OnEventLoopEvent(const LinuxEventLoopEvent &event) noexcept
      -> Result<core::Void>;
  [[nodiscard]] auto OnEpollEvent(const struct epoll_event &event) noexcept
      -> Result<core::Void>;

  core::Tx<LinuxEventLoopEvent> main_to_lobby_tx_;
  LinuxEventLoop event_loop_;
  LinuxFileDescriptor server_fd_;

  friend class LinuxMainEventLoopBuilder;
};

class LinuxMainEventLoopBuilder final : private core::NonCopyable,
                                        core::NonMovable {
public:
  [[nodiscard]] auto
  Build(const uint16_t port, core::Rx<LinuxEventLoopEvent> &&signal_to_main_rx,
        core::Tx<LinuxEventLoopEvent> &&main_to_lobby_tx) const noexcept
      -> Result<LinuxMainEventLoop>;
};

#endif // SERVER_MAIN_EVENT_LOOP_LINUX_H
