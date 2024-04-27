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
    -> Result<Void> {
  return UpdateNonBlocking(fd_);
}

auto engine::FileDescriptorLinux::IsValid(const Raw fd) noexcept -> bool {
  return fd != kInvalidFd;
}

auto engine::FileDescriptorLinux::UpdateNonBlocking(const Raw fd) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

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

  return ResultT{Void{}};
}

auto engine::FileDescriptorLinux::ParseFdToSessionId(const Raw fd) noexcept
    -> Result<SessionId> {
  using ResultT = Result<SessionId>;

  const auto casted_session_id = static_cast<SessionId>(fd);
  const auto restored_fd = static_cast<Raw>(casted_session_id);
  if (fd != restored_fd) {
    return ResultT{Error{Symbol::kFileDescriptorLinuxParseFdToSessionIdFailed,
                         core::TinyJson{}
                             .Set("fd", fd)
                             .Set("casted_session_id", casted_session_id)
                             .Set("restored_fd", restored_fd)
                             .ToString()}};
  }

  return ResultT{casted_session_id};
}

auto engine::FileDescriptorLinux::ParseSessionIdToFd(
    const SessionId session_id) noexcept -> Result<FileDescriptorLinux::Raw> {
  using ResultT = Result<FileDescriptorLinux::Raw>;

  const auto casted_fd = static_cast<Raw>(session_id);
  const auto restored_session_id = static_cast<SessionId>(casted_fd);
  if (session_id != restored_session_id) {
    return ResultT{Error{Symbol::kFileDescriptorLinuxParseSessionIdToFdFailed,
                         core::TinyJson{}
                             .Set("session_id", session_id)
                             .Set("casted_fd", casted_fd)
                             .Set("restored_session_id", restored_session_id)
                             .ToString()}};
  }

  return ResultT{casted_fd};
}

auto engine::FileDescriptorLinux::FromSessionId(
    const SessionId session_id) noexcept -> Result<FileDescriptorLinux> {
  using ResultT = Result<FileDescriptorLinux>;

  auto res = ParseSessionIdToFd(session_id);
  if (res.IsErr()) {
    return ResultT{std::move(res.Err())};
  }

  return ResultT{FileDescriptorLinux{res.Ok()}};
}

auto engine::operator<<(std::ostream &os, const FileDescriptorLinux &fd)
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
