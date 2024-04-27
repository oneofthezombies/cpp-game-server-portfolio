#ifndef SERVER_ENGINE_EVENT_LOOP_LINUX_H
#define SERVER_ENGINE_EVENT_LOOP_LINUX_H

#include <sys/epoll.h>

#include "core/core.h"

#include "event_loop.h"
#include "event_loop_handler.h"
#include "file_descriptor_linux.h"
#include "mail_center.h"

namespace engine {

class EventLoopLinux final {
public:
  class Builder {
  public:
    explicit Builder() noexcept = default;
    ~Builder() noexcept = default;
    CLASS_KIND_PINNABLE(Builder);

    [[nodiscard]] auto Build(std::string &&name,
                             EventLoopHandlerPtr &&handler) const noexcept
        -> Result<EventLoopLinux>;
  };

  ~EventLoopLinux() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoopLinux);

  [[nodiscard]] auto Init(const Config &config) noexcept -> Result<Void>;
  [[nodiscard]] auto Run() noexcept -> Result<Void>;
  [[nodiscard]] auto Name() const noexcept -> std::string_view;

  [[nodiscard]] auto Add(const SessionId session_id,
                         const uint32_t events) const noexcept -> Result<Void>;

  [[nodiscard]] auto Delete(const FileDescriptorLinux::Raw fd) noexcept
      -> Result<Void>;
  [[nodiscard]] auto Write(const FileDescriptorLinux::Raw fd,
                           const std::string_view data) noexcept
      -> Result<Void>;

  [[nodiscard]] auto OnMailReceived(const Mail &mail) noexcept -> Result<Void>;
  [[nodiscard]] auto
  OnEpollEventReceived(const struct epoll_event &event) noexcept
      -> Result<Void>;

protected:
  EventLoopContext context_;
  EventLoopHandlerPtr handler_;
  FileDescriptorLinux epoll_fd_;

  static constexpr size_t kMaxEvents = 1024;

private:
  explicit EventLoopLinux(EventLoopContext &&context,
                          EventLoopHandlerPtr &&handler,
                          FileDescriptorLinux &&epoll_fd) noexcept;

  [[nodiscard]] auto
  ParseSessionIdToFd(const SessionId session_id) const noexcept
      -> Result<FileDescriptorLinux::Raw>;
};

} // namespace engine

#endif // SERVER_ENGINE_EVENT_LOOP_LINUX_H
