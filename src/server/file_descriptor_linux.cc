#include "file_descriptor_linux.h"

#include <cstring>
#include <fcntl.h>
#include <unistd.h>

LinuxFileDescriptor::LinuxFileDescriptor(const int fd) noexcept : fd_{fd} {}

LinuxFileDescriptor::LinuxFileDescriptor(LinuxFileDescriptor &&other) noexcept
    : fd_{other.fd_} {
  other.fd_ = kInvalidFd;
}

LinuxFileDescriptor::~LinuxFileDescriptor() noexcept {
  if (fd_ == kInvalidFd) {
    return;
  }

  close(fd_);
  fd_ = kInvalidFd;
}

auto LinuxFileDescriptor::operator=(LinuxFileDescriptor &&other) noexcept
    -> LinuxFileDescriptor & {
  if (this == &other) {
    return *this;
  }

  fd_ = other.fd_;
  other.fd_ = kInvalidFd;

  return *this;
}

auto LinuxFileDescriptor::AsRaw() const noexcept -> int { return fd_; }

auto LinuxFileDescriptor::IsValid() const noexcept -> bool {
  return fd_ != kInvalidFd;
}

auto LinuxFileDescriptor::UpdateNonBlocking() const noexcept
    -> Result<core::Void> {
  const int opts = fcntl(fd_, F_GETFL);
  if (opts < 0) {
    return Error{Symbol::kLinuxFileDescriptorGetStatusFailed,
                 std::string{strerror(errno)}};
  }

  if (fcntl(fd_, F_SETFL, (opts | O_NONBLOCK)) < 0) {
    return Error{Symbol::kLinuxFileDescriptorSetStatusFailed,
                 std::string{strerror(errno)}};
  }

  return core::Void{};
}
