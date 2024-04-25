#include "file_descriptor_linux.h"

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
        SB{}.Add(core::LinuxError::FromErrno()).Add("fd", fd_).Build()};
    std::cout << error << std::endl;
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
  return IsValid(fd_);
}

auto LinuxFileDescriptor::UpdateNonBlocking() const noexcept
    -> Result<core::Void> {
  return UpdateNonBlocking(fd_);
}

auto LinuxFileDescriptor::IsValid(const Raw fd) noexcept -> bool {
  return fd != kInvalidFd;
}

auto LinuxFileDescriptor::UpdateNonBlocking(const Raw fd) noexcept
    -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  const int opts = fcntl(fd, F_GETFL);
  if (opts < 0) {
    return ResultT{
        Error{Symbol::kLinuxFileDescriptorGetStatusFailed,
              SB{}.Add(core::LinuxError::FromErrno()).Add("fd", fd).Build()}};
  }

  if (fcntl(fd, F_SETFL, (opts | O_NONBLOCK)) < 0) {
    return ResultT{
        Error{Symbol::kLinuxFileDescriptorSetStatusFailed,
              SB{}.Add(core::LinuxError::FromErrno()).Add("fd", fd).Build()}};
  }

  return ResultT{core::Void{}};
}

auto LinuxFileDescriptor::RawToSessionId(const Raw fd) noexcept
    -> Session::IdType {
  return static_cast<Session::IdType>(fd);
}

auto operator<<(std::ostream &os, const LinuxFileDescriptor &fd)
    -> std::ostream & {
  os << "LinuxFileDescriptor{";
  os << "fd=";
  if (fd.IsValid()) {
    os << fd.AsRaw();
  } else {
    os << "invalid";
  }
  os << "}";
  return os;
}
