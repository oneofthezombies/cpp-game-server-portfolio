#include "file_descriptor_linux.h"

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#include "core/utils.h"
#include "core/utils_linux.h"

LinuxFileDescriptor::LinuxFileDescriptor(const Raw fd) noexcept : fd_{fd} {}

LinuxFileDescriptor::LinuxFileDescriptor(LinuxFileDescriptor &&other) noexcept
    : fd_{other.fd_} {
  other.fd_ = kInvalidFd;
}

LinuxFileDescriptor::~LinuxFileDescriptor() noexcept {
  if (fd_ == kInvalidFd) {
    return;
  }

  if (close(fd_) == -1) {
    const Error error{
        Symbol::kLinuxFileDescriptorCloseFailed,
        core::StringBuilder{}.Add(core::LinuxError::FromErrno()).Build()};
    // TODO: Log error
    std::cout << error.message << std::endl;
  }

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

auto LinuxFileDescriptor::AsRaw() const noexcept -> Raw { return fd_; }

auto LinuxFileDescriptor::IsValid() const noexcept -> bool {
  return fd_ != kInvalidFd;
}

auto LinuxFileDescriptor::UpdateNonBlocking() const noexcept
    -> Result<core::Void> {
  const int opts = fcntl(fd_, F_GETFL);
  if (opts < 0) {
    return Error{
        Symbol::kLinuxFileDescriptorGetStatusFailed,
        core::StringBuilder{}.Add(core::LinuxError::FromErrno()).Build()};
  }

  if (fcntl(fd_, F_SETFL, (opts | O_NONBLOCK)) < 0) {
    return Error{
        Symbol::kLinuxFileDescriptorSetStatusFailed,
        core::StringBuilder{}.Add(core::LinuxError::FromErrno()).Build()};
  }

  return core::Void{};
}
