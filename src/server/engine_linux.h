#ifndef SERVER_ENGINE_LINUX_H
#define SERVER_ENGINE_LINUX_H

#include "file_descriptor_linux.h"

class LinuxEngine final : private core::NonCopyable, core::Movable {
public:
  LinuxEngine(LinuxEngine &&) noexcept = default;
  ~LinuxEngine() noexcept = default;

  auto operator=(LinuxEngine &&) noexcept -> LinuxEngine & = default;

  [[nodiscard]] auto Run() noexcept -> Result<core::Void>;

private:
  LinuxEngine(LinuxFileDescriptor &&epoll_fd,
              LinuxFileDescriptor &&server_fd) noexcept;

  LinuxFileDescriptor epoll_fd_;
  LinuxFileDescriptor server_fd_;

  static constexpr size_t kMaxEvents = 64;

  friend class LinuxEngineBuilder;
};

class LinuxEngineBuilder final : private core::NonCopyable, core::NonMovable {
public:
  [[nodiscard]] auto Build(const uint16_t port) const noexcept
      -> Result<LinuxEngine>;
};

#endif // SERVER_ENGINE_LINUX_H
