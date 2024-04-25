#include "file_descriptor_linux.h"

#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#include "core/utils.h"
#include "core/utils_linux.h"

FileDescriptorLinux::FileDescriptorLinux(const Raw fd) noexcept : fd_{fd} {}

FileDescriptorLinux::FileDescriptorLinux(FileDescriptorLinux &&other) noexcept
    : fd_{other.fd_} {
  other.fd_ = kInvalidFd;
}

FileDescriptorLinux::~FileDescriptorLinux() noexcept {
  if (fd_ == kInvalidFd) {
    return;
  }

  if (close(fd_) == -1) {
    const Error error{
        Symbol::kFileDescriptorLinuxCloseFailed,
        SB{}.Add(core::LinuxError::FromErrno()).Add("fd", fd_).Build()};
    std::cout << error << std::endl;
  }

  fd_ = kInvalidFd;
}

auto FileDescriptorLinux::operator=(FileDescriptorLinux &&other) noexcept
    -> FileDescriptorLinux & {
  if (this == &other) {
    return *this;
  }

  fd_ = other.fd_;
  other.fd_ = kInvalidFd;

  return *this;
}

auto FileDescriptorLinux::AsRaw() const noexcept -> Raw { return fd_; }

auto FileDescriptorLinux::IsValid() const noexcept -> bool {
  return IsValid(fd_);
}

auto FileDescriptorLinux::UpdateNonBlocking() const noexcept
    -> Result<core::Void> {
  return UpdateNonBlocking(fd_);
}

auto FileDescriptorLinux::IsValid(const Raw fd) noexcept -> bool {
  return fd != kInvalidFd;
}

auto FileDescriptorLinux::UpdateNonBlocking(const Raw fd) noexcept
    -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  const int opts = fcntl(fd, F_GETFL);
  if (opts < 0) {
    return ResultT{
        Error{Symbol::kFileDescriptorLinuxGetStatusFailed,
              SB{}.Add(core::LinuxError::FromErrno()).Add("fd", fd).Build()}};
  }

  if (fcntl(fd, F_SETFL, (opts | O_NONBLOCK)) < 0) {
    return ResultT{
        Error{Symbol::kFileDescriptorLinuxSetStatusFailed,
              SB{}.Add(core::LinuxError::FromErrno()).Add("fd", fd).Build()}};
  }

  return ResultT{core::Void{}};
}

auto FileDescriptorLinux::RawToSessionId(const Raw fd) noexcept
    -> Session::IdType {
  return static_cast<Session::IdType>(fd);
}

auto operator<<(std::ostream &os, const FileDescriptorLinux &fd)
    -> std::ostream & {
  os << "FileDescriptorLinux{";
  os << "fd=";
  if (fd.IsValid()) {
    os << fd.AsRaw();
  } else {
    os << "invalid";
  }
  os << "}";
  return os;
}
