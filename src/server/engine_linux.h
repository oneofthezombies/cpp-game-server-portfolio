#ifndef SERVER_ENGINE_LINUX_H
#define SERVER_ENGINE_LINUX_H

#include <thread>

#include "file_descriptor_linux.h"
#include "server/main_event_loop_linux.h"

class EngineLinux final : private NonCopyable, Movable {
public:
  [[nodiscard]] auto Run() noexcept -> Result<Void>;

private:
  EngineLinux(MainEventLoopLinux &&main_event_loop, std::thread &&lobby_thread,
              std::thread &&battle_thread) noexcept;

  [[nodiscard]] auto OnServerFdEvent() noexcept -> Result<Void>;
  [[nodiscard]] auto
  OnClientFdEvent(const FileDescriptorLinux::Raw client_fd) noexcept
      -> Result<Void>;

  auto DeleteConnectedSessionOrCloseFd(
      const FileDescriptorLinux::Raw client_fd) noexcept -> void;

  MainEventLoopLinux main_event_loop_;
  std::thread lobby_thread_;
  std::thread battle_thread_;

  friend class EngineLinuxBuilder;
};

class EngineLinuxBuilder final : private NonCopyable, NonMovable {
public:
  [[nodiscard]] auto Build(const uint16_t port) const noexcept
      -> Result<EngineLinux>;
};

#endif // SERVER_ENGINE_LINUX_H
