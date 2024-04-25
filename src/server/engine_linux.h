#ifndef SERVER_ENGINE_LINUX_H
#define SERVER_ENGINE_LINUX_H

#include <thread>

#include "file_descriptor_linux.h"
#include "server/main_event_loop_linux.h"

class LinuxEngine final : private core::NonCopyable, core::Movable {
public:
  [[nodiscard]] auto Run() noexcept -> Result<core::Void>;

private:
  LinuxEngine(LinuxMainEventLoop &&main_event_loop,
              core::Tx<LinuxEventLoopEvent> &&signal_to_main_tx,
              std::thread &&lobby_thread) noexcept;

  [[nodiscard]] auto OnServerFdEvent() noexcept -> Result<core::Void>;
  [[nodiscard]] auto
  OnClientFdEvent(const LinuxFileDescriptor::Raw client_fd) noexcept
      -> Result<core::Void>;

  auto DeleteConnectedSessionOrCloseFd(
      const LinuxFileDescriptor::Raw client_fd) noexcept -> void;

  LinuxMainEventLoop main_event_loop_;
  core::Tx<LinuxEventLoopEvent> signal_to_main_tx_;
  std::thread lobby_thread_;

  friend class LinuxEngineBuilder;
};

class LinuxEngineBuilder final : private core::NonCopyable, core::NonMovable {
public:
  [[nodiscard]] auto Build(const uint16_t port) const noexcept
      -> Result<LinuxEngine>;
};

#endif // SERVER_ENGINE_LINUX_H
