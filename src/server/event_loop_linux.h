#ifndef SERVER_EVENT_LOOP_LINUX_H
#define SERVER_EVENT_LOOP_LINUX_H

#include <functional>

#include "core/core.h"
#include "core/spsc_channel.h"

#include "file_descriptor_linux.h"

using LinuxEventLoopEvent = std::unordered_map<std::string, std::string>;

using OnEventLoopEvent =
    std::function<Result<core::Void>(const LinuxEventLoopEvent &)>;
using OnEpollEvent =
    std::function<Result<core::Void>(const struct epoll_event &)>;

class LinuxEventLoop final : private core::NonCopyable, core::Movable {
public:
  [[nodiscard]] auto Add(const LinuxFileDescriptor::Raw fd,
                         uint32_t events) noexcept -> Result<core::Void>;
  [[nodiscard]] auto Delete(const LinuxFileDescriptor::Raw fd,
                            uint32_t events) noexcept -> Result<core::Void>;
  [[nodiscard]] auto Run(OnEventLoopEvent &&on_event_loop_event,
                         OnEpollEvent &&on_epoll_event) noexcept
      -> Result<core::Void>;

private:
  explicit LinuxEventLoop(core::Rx<LinuxEventLoopEvent> &&event_loop_rx,
                          LinuxFileDescriptor &&epoll_fd) noexcept;

  core::Rx<LinuxEventLoopEvent> event_loop_rx_;
  LinuxFileDescriptor epoll_fd_;

  static constexpr size_t kMaxEvents = 1024;

  friend class LinuxEventLoopBuilder;
};

class LinuxEventLoopBuilder final : private core::NonCopyable,
                                    core::NonMovable {
public:
  [[nodiscard]] auto
  Build(core::Rx<LinuxEventLoopEvent> &&event_loop_rx) const noexcept
      -> Result<LinuxEventLoop>;
};

#endif // SERVER_EVENT_LOOP_LINUX_H
