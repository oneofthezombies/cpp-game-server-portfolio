#ifndef SERVER_MAIN_EVENT_LOOP_LINUX_H
#define SERVER_MAIN_EVENT_LOOP_LINUX_H

#include "core/core.h"

#include "event_loop_linux.h"
#include "server/file_descriptor_linux.h"

class MainEventLoopLinux final : private NonCopyable, Movable {
public:
  [[nodiscard]] auto Run() noexcept -> Result<Void>;

private:
  explicit MainEventLoopLinux(Tx<EventLoopLinuxEvent> &&main_to_lobby_tx,
                              EventLoopLinux &&event_loop,
                              FileDescriptorLinux &&server_fd) noexcept;

  [[nodiscard]] auto OnEventLoopEvent(const EventLoopLinuxEvent &event) noexcept
      -> Result<Void>;
  [[nodiscard]] auto OnEpollEvent(const struct epoll_event &event) noexcept
      -> Result<Void>;

  Tx<EventLoopLinuxEvent> main_to_lobby_tx_;
  EventLoopLinux event_loop_;
  FileDescriptorLinux server_fd_;

  friend class MainEventLoopLinuxBuilder;
};

class MainEventLoopLinuxBuilder final : private NonCopyable, NonMovable {
public:
  [[nodiscard]] auto Build(const uint16_t port) const noexcept
      -> Result<MainEventLoopLinux>;
};

#endif // SERVER_MAIN_EVENT_LOOP_LINUX_H
