#ifndef SERVER_ENGINE_MAIN_EVENT_LOOP_LINUX_H
#define SERVER_ENGINE_MAIN_EVENT_LOOP_LINUX_H

#include "core/core.h"

#include "event_loop_linux.h"
#include "file_descriptor_linux.h"

namespace engine {

class MainEventLoopLinux final : public EventLoopLinux {
public:
  using Super = EventLoopLinux;

  class Builder final {
  public:
    explicit Builder() noexcept = default;
    ~Builder() noexcept = default;
    CLASS_KIND_PINNABLE(Builder);

    [[nodiscard]] auto Build(const uint16_t port) const noexcept
        -> Result<MainEventLoopLinux>;
  };

  ~MainEventLoopLinux() noexcept = default;
  CLASS_KIND_MOVABLE(MainEventLoopLinux);

private:
  explicit MainEventLoopLinux(FileDescriptorLinux &&server_fd) noexcept;

  [[nodiscard]] auto OnMailReceived(const Mail &mail) noexcept
      -> Result<Void> override;

  [[nodiscard]] auto
  OnEpollEventReceived(const struct epoll_event &event) noexcept
      -> Result<Void> override;

  FileDescriptorLinux server_fd_;
};

} // namespace engine

#endif // SERVER_ENGINE_MAIN_EVENT_LOOP_LINUX_H
