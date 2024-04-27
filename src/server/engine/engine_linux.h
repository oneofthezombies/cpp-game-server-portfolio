#ifndef SERVER_ENGINE_ENGINE_LINUX_H
#define SERVER_ENGINE_ENGINE_LINUX_H

#include <thread>
#include <unordered_map>

#include "config.h"
#include "event_loop.h"
#include "file_descriptor_linux.h"

namespace engine {

class EngineLinux final {
public:
  class Builder final {
  public:
    explicit Builder() noexcept = default;
    ~Builder() noexcept = default;
    CLASS_KIND_PINNABLE(Builder);

    [[nodiscard]] auto Build(Config &&config) const noexcept
        -> Result<EngineLinux>;
  };

  ~EngineLinux() noexcept = default;
  CLASS_KIND_MOVABLE(EngineLinux);

  [[nodiscard]] auto Init() noexcept -> Result<Void>;
  [[nodiscard]] auto Run() noexcept -> Result<Void>;

  [[nodiscard]] auto AddEventLoop(std::string &&name,
                                  EventLoopHandlerPtr &&handler) noexcept
      -> Result<Void>;

private:
  explicit EngineLinux(Config &&config,
                       EventLoopPtr &&main_event_loop) noexcept;

  [[nodiscard]] auto OnServerFdEvent() noexcept -> Result<Void>;
  [[nodiscard]] auto
  OnClientFdEvent(const FileDescriptorLinux::Raw client_fd) noexcept
      -> Result<Void>;

  auto DeleteConnectedSessionOrCloseFd(
      const FileDescriptorLinux::Raw client_fd) noexcept -> void;

  static auto EventLoopThreadMain(EventLoopPtr &&event_loop) noexcept -> void;

  std::unordered_map<std::string, std::thread> event_loop_threads_;
  EventLoopPtr main_event_loop_;
  Config config_;
};

} // namespace engine

#endif // SERVER_ENGINE_ENGINE_LINUX_H
