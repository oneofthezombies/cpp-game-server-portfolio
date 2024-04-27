#ifndef SERVER_ENGINE_MAIN_EVENT_LOOP_HANDLER_LINUX_H
#define SERVER_ENGINE_MAIN_EVENT_LOOP_HANDLER_LINUX_H

#include "event_loop.h"
#include "file_descriptor_linux.h"

namespace engine {

class MainEventLoopHandlerLinux final : public EventLoopHandler {
public:
  explicit MainEventLoopHandlerLinux(
      std::string &&primary_event_loop_name) noexcept;
  virtual ~MainEventLoopHandlerLinux() noexcept override = default;
  CLASS_KIND_MOVABLE(MainEventLoopHandlerLinux);

  [[nodiscard]] virtual auto OnInit(const EventLoop &event_loop,
                                    const Config &config) noexcept
      -> Result<Void> override;

  [[nodiscard]] virtual auto OnMail(const EventLoop &event_loop,
                                    const Mail &mail) noexcept
      -> Result<Void> override;

  [[nodiscard]] virtual auto OnSocketIn(const EventLoop &event_loop,
                                        const SocketId socket_id) noexcept
      -> Result<Void> override;

  [[nodiscard]] virtual auto OnSocketHangUp(const EventLoop &event_loop,
                                            const SocketId socket_id) noexcept
      -> Result<Void> override;

  [[nodiscard]] virtual auto
  OnSocketError(const EventLoop &event_loop, const SocketId socket_id,
                const int code, const std::string_view description) noexcept
      -> Result<Void> override;

private:
  std::unique_ptr<FileDescriptorLinux> server_fd_;
  std::string primary_event_loop_name_;
};

} // namespace engine

#endif // SERVER_ENGINE_MAIN_EVENT_LOOP_HANDLER_LINUX_H
