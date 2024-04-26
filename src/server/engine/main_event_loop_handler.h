#ifndef SERVER_ENGINE_MAIN_EVENT_LOOP_HANDLER_H
#define SERVER_ENGINE_MAIN_EVENT_LOOP_HANDLER_H

#include "event_loop_handler.h"
#include "file_descriptor_linux.h"

namespace engine {

class MainEventLoopHandler final : public EventLoopHandler {
public:
  explicit MainEventLoopHandler() noexcept = default;
  virtual ~MainEventLoopHandler() noexcept override = default;
  CLASS_KIND_MOVABLE(MainEventLoopHandler);

  [[nodiscard]] virtual auto OnInit(const Config &config,
                                    const EventLoop &event_loop) noexcept
      -> Result<Void> override;

private:
  std::unique_ptr<FileDescriptorLinux> server_fd_;
};

} // namespace engine

#endif // SERVER_ENGINE_MAIN_EVENT_LOOP_HANDLER_H
