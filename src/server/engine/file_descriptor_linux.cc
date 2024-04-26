#include "file_descriptor_linux.h"

#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#include "core/tiny_json.h"
#include "core/utils_linux.h"

using namespace engine;

engine::FileDescriptorLinux::FileDescriptorLinux(const Raw fd) noexcept
    : fd_{fd} {}

engine::FileDescriptorLinux::FileDescriptorLinux(
    FileDescriptorLinux &&other) noexcept
    : fd_{other.fd_} {
  other.fd_ = kInvalidFd;
}

engine::FileDescriptorLinux::~FileDescriptorLinux() noexcept {
  if (fd_ == kInvalidFd) {
    return;
  }

  if (close(fd_) == -1) {
    core::TinyJson{}
        .Set("symbol", Symbol::kFileDescriptorLinuxCloseFailed)
        .Set("linux_error", core::LinuxError::FromErrno())
        .Set("fd", fd_)
        .LogLn();
  }

  fd_ = kInvalidFd;
}

auto engine::FileDescriptorLinux::operator=(
    FileDescriptorLinux &&other) noexcept -> FileDescriptorLinux & {
  if (this == &other) {
    return *this;
  }

  fd_ = other.fd_;
  other.fd_ = kInvalidFd;

  return *this;
}

auto engine::FileDescriptorLinux::AsRaw() const noexcept -> Raw { return fd_; }

auto engine::FileDescriptorLinux::IsValid() const noexcept -> bool {
  return IsValid(fd_);
}

auto engine::FileDescriptorLinux::UpdateNonBlocking() const noexcept
    -> Result<core::Void> {
  return UpdateNonBlocking(fd_);
}

auto engine::FileDescriptorLinux::IsValid(const Raw fd) noexcept -> bool {
  return fd != kInvalidFd;
}

auto engine::FileDescriptorLinux::UpdateNonBlocking(const Raw fd) noexcept
    -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  const int opts = fcntl(fd, F_GETFL);
  if (opts < 0) {
    return ResultT{Error{Symbol::kFileDescriptorLinuxGetStatusFailed,
                         core::TinyJson{}
                             .Set("linux_error", core::LinuxError::FromErrno())
                             .Set("fd", fd)
                             .ToString()}};
  }

  if (fcntl(fd, F_SETFL, (opts | O_NONBLOCK)) < 0) {
    return ResultT{Error{Symbol::kFileDescriptorLinuxSetStatusFailed,
                         core::TinyJson{}
                             .Set("linux_error", core::LinuxError::FromErrno())
                             .Set("fd", fd)
                             .ToString()}};
  }

  return ResultT{core::Void{}};
}

auto engine::operator<<(std::ostream &os,
                        const FileDescriptorLinux &fd) -> std::ostream & {
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
