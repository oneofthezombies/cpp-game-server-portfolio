#ifndef SERVER_FILE_DESCRIPTOR_LINUX_H
#define SERVER_FILE_DESCRIPTOR_LINUX_H

#include "core/core.h"

#include "common.h"

class LinuxFileDescriptor final : private core::NonCopyable, core::Movable {
public:
  explicit LinuxFileDescriptor(const int fd) noexcept;
  LinuxFileDescriptor(LinuxFileDescriptor &&other) noexcept;
  ~LinuxFileDescriptor() noexcept;

  auto operator=(LinuxFileDescriptor &&other) noexcept -> LinuxFileDescriptor &;

  auto IsValid() const noexcept -> bool;
  auto AsRaw() const noexcept -> int;

  [[nodiscard]] auto UpdateNonBlocking() const noexcept -> Result<core::Void>;

private:
  int fd_{kInvalidFd};

  static constexpr int kInvalidFd{-1};
};

#endif // SERVER_FILE_DESCRIPTOR_LINUX_H
