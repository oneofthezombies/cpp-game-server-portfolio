#ifndef SERVER_ENGINE_EVENT_LOOP_LINUX_H
#define SERVER_ENGINE_EVENT_LOOP_LINUX_H

#include <sys/epoll.h>

#include "core/core.h"

#include "event_loop.h"
#include "file_descriptor_linux.h"

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
  [[nodiscard]] virtual auto Add(const SocketId socket_id,
                                 const uint32_t events) const noexcept
      -> Result<Void> override;
  [[nodiscard]] auto Delete(const SocketId socket_id) const noexcept
      -> Result<Void> override;
  [[nodiscard]] auto Write(const SocketId socket_id,
                           const std::string_view data) const noexcept
      -> Result<Void> override;

private:
  FileDescriptorLinux epoll_fd_;

  static constexpr size_t kMaxEvents = 1024;

private:
  explicit EventLoopLinux(MailBox &&mail_box, std::string &&name,
                          EventLoopHandlerPtr &&handler,
                          FileDescriptorLinux &&epoll_fd) noexcept;
};

} // namespace engine

#endif // SERVER_ENGINE_EVENT_LOOP_LINUX_H
