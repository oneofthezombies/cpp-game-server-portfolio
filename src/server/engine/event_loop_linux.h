#ifndef SERVER_ENGINE_EVENT_LOOP_LINUX_H
#define SERVER_ENGINE_EVENT_LOOP_LINUX_H

#include <sys/epoll.h>

#include "core/core.h"

#include "event_loop.h"
#include "file_descriptor_linux.h"
#include "mail_center.h"

namespace engine {

class EventLoopLinux final : public EventLoop {
public:
  class Builder {
  public:
    explicit Builder() noexcept = default;
    ~Builder() noexcept = default;
    CLASS_KIND_PINNABLE(Builder);

    [[nodiscard]] auto Build(std::string &&name,
                             EventLoopHandlerPtr &&handler) const noexcept
        -> Result<EventLoopPtr>;
  };

  virtual ~EventLoopLinux() noexcept override = default;
  CLASS_KIND_MOVABLE(EventLoopLinux);

  [[nodiscard]] virtual auto Init(const Config &config) noexcept
      -> Result<Void> override;
  [[nodiscard]] virtual auto Run() noexcept -> Result<Void> override;
  [[nodiscard]] virtual auto Name() const noexcept -> std::string_view override;

  [[nodiscard]] virtual auto Add(const SessionId session_id,
                                 const uint32_t events) const noexcept
      -> Result<Void> override;

  [[nodiscard]] auto Delete(const FileDescriptorLinux::Raw fd) noexcept
      -> Result<Void>;
  [[nodiscard]] auto Write(const FileDescriptorLinux::Raw fd,
                           const std::string_view data) noexcept
      -> Result<Void>;

  [[nodiscard]] auto OnMailReceived(const Mail &mail) noexcept -> Result<Void>;
  [[nodiscard]] auto
  OnEpollEventReceived(const struct epoll_event &event) noexcept
      -> Result<Void>;

private:
  FileDescriptorLinux epoll_fd_;

  static constexpr size_t kMaxEvents = 1024;

private:
  explicit EventLoopLinux(EventLoopContext &&context,
                          EventLoopHandlerPtr &&handler,
                          FileDescriptorLinux &&epoll_fd) noexcept;
};

} // namespace engine

#endif // SERVER_ENGINE_EVENT_LOOP_LINUX_H
