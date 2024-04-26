#ifndef SERVER_ENGINE_EVENT_LOOP_LINUX_H
#define SERVER_ENGINE_EVENT_LOOP_LINUX_H

#include <sys/epoll.h>

#include "core/core.h"

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

  [[nodiscard]] auto Run() noexcept -> Result<Void>;

  [[nodiscard]] auto Add(const FileDescriptorLinux::Raw fd,
                         const uint32_t events) noexcept -> Result<Void>;
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
  MailBox mail_box_;
  std::string name_;
  FileDescriptorLinux epoll_fd_;
  EventLoopHandlerPtr handler_;

  static constexpr size_t kMaxEvents = 1024;

private:
  explicit EventLoopLinux(MailBox &&mail_box, std::string &&name,
                          FileDescriptorLinux &&epoll_fd,
                          EventLoopHandlerPtr &&handler) noexcept;
};

} // namespace engine

#endif // SERVER_ENGINE_EVENT_LOOP_LINUX_H
