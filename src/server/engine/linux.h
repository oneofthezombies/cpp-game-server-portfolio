#ifndef CPP_GAME_SERVER_PORTFOLIO_SERVER_SERVER_ENGINE_LINUX_H
#define CPP_GAME_SERVER_PORTFOLIO_SERVER_SERVER_ENGINE_LINUX_H

#include "core/core.h"

#include "../engine.h"

class FileDescriptor final : private core::NonCopyable, core::Movable {
public:
  explicit FileDescriptor(const int file_descriptor) noexcept;
  ~FileDescriptor() noexcept;

  auto IsValid() const noexcept -> bool;

  auto Get() const noexcept -> const int;

private:
  int file_descriptor_{kInvalidFileDescriptor};

  static constexpr int kInvalidFileDescriptor{-1};
};

class LinuxEngine final : private core::NonCopyable, core::Movable {

private:
  LinuxEngine(FileDescriptor &&epoll_fd) noexcept;

  FileDescriptor epoll_fd_;

  friend class LinuxEngineFactory;
};

class LinuxEngineFactory final : private core::NonCopyable, core::NonMovable {
public:
  auto Create() const noexcept -> Result<LinuxEngine>;
};

#endif // CPP_GAME_SERVER_PORTFOLIO_SERVER_SERVER_ENGINE_LINUX_H
