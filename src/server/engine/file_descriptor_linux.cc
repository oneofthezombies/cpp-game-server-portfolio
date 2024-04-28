#include "file_descriptor_linux.h"

#include <fcntl.h>
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
  if (auto res = Close(fd_); res.IsErr()) {
    core::TinyJson{}
        .Set("message", "file descriptor close failed")
        .Set("error", res.Err())
        .LogLn();
    return;
  }

  fd_ = kInvalidFd;
}

auto
engine::FileDescriptorLinux::operator=(FileDescriptorLinux &&other) noexcept
    -> FileDescriptorLinux & {
  if (this == &other) {
    return *this;
  }

  fd_ = other.fd_;
  other.fd_ = kInvalidFd;

  return *this;
}

auto
engine::FileDescriptorLinux::AsRaw() const noexcept -> Raw {
  return fd_;
}

auto
engine::FileDescriptorLinux::IsValid() const noexcept -> bool {
  return IsValid(fd_);
}

auto
engine::FileDescriptorLinux::UpdateNonBlocking() const noexcept
    -> Result<Void> {
  return UpdateNonBlocking(fd_);
}

auto
engine::FileDescriptorLinux::IsValid(const Raw fd) noexcept -> bool {
  return fd != kInvalidFd;
}

auto
engine::FileDescriptorLinux::UpdateNonBlocking(const Raw fd) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  const int opts = fcntl(fd, F_GETFL);
  if (opts < 0) {
    return ResultT{
        Error::From(kFileDescriptorLinuxGetStatusFailed,
                    core::TinyJson{}
                        .Set("linux_error", core::LinuxError::FromErrno())
                        .Set("fd", fd)
                        .IntoMap())};
  }

  if (fcntl(fd, F_SETFL, (opts | O_NONBLOCK)) < 0) {
    return ResultT{
        Error::From(kFileDescriptorLinuxSetStatusFailed,
                    core::TinyJson{}
                        .Set("linux_error", core::LinuxError::FromErrno())
                        .Set("fd", fd)
                        .IntoMap())};
  }

  return ResultT{Void{}};
}

auto
engine::FileDescriptorLinux::ParseFdToSocketId(const Raw fd) noexcept
    -> Result<SocketId> {
  using ResultT = Result<SocketId>;

  const auto casted_socket_id = static_cast<SocketId>(fd);
  const auto restored_fd = static_cast<Raw>(casted_socket_id);
  if (fd != restored_fd) {
    return ResultT{Error::From(kFileDescriptorLinuxParseFdToSocketIdFailed,
                               core::TinyJson{}
                                   .Set("fd", fd)
                                   .Set("casted_socket_id", casted_socket_id)
                                   .Set("restored_fd", restored_fd)
                                   .IntoMap())};
  }

  return ResultT{casted_socket_id};
}

auto
engine::FileDescriptorLinux::ParseSocketIdToFd(
    const SocketId socket_id) noexcept -> Result<FileDescriptorLinux::Raw> {
  using ResultT = Result<FileDescriptorLinux::Raw>;

  const auto casted_fd = static_cast<Raw>(socket_id);
  const auto restored_socket_id = static_cast<SocketId>(casted_fd);
  if (socket_id != restored_socket_id) {
    return ResultT{
        Error::From(kFileDescriptorLinuxParseSocketIdToFdFailed,
                    core::TinyJson{}
                        .Set("socket_id", socket_id)
                        .Set("casted_fd", casted_fd)
                        .Set("restored_socket_id", restored_socket_id)
                        .IntoMap())};
  }

  return ResultT{casted_fd};
}

auto
engine::FileDescriptorLinux::Close(const Raw fd) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (fd == kInvalidFd) {
    return ResultT{Void{}};
  }

  if (close(fd) == -1) {
    return ResultT{
        Error::From(kFileDescriptorLinuxCloseFailed,
                    core::TinyJson{}
                        .Set("linux_error", core::LinuxError::FromErrno())
                        .Set("fd", fd)
                        .IntoMap())};
  }

  return ResultT{Void{}};
}

auto
engine::operator<<(std::ostream &os,
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
