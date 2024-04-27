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

  [[nodiscard]] virtual auto OnInit(const Config &config,
                                    const EventLoop &event_loop) noexcept
      -> Result<Void> override;

  [[nodiscard]] virtual auto OnMail(const EventLoopContext &context,
                                    Mail &&mail) noexcept
      -> Result<Void> override;

  [[nodiscard]] virtual auto OnSessionEvent(const EventLoopContext &context,
                                            const SessionId session_id,
                                            const uint32_t events) noexcept
      -> Result<Void> override;

private:
  std::unique_ptr<FileDescriptorLinux> server_fd_;
  std::string primary_event_loop_name_;
};

} // namespace engine

#endif // SERVER_ENGINE_MAIN_EVENT_LOOP_HANDLER_LINUX_H
