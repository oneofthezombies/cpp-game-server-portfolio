#if defined(__linux__)
#include "linux.h"

#include <cassert>

#include <sys/epoll.h>
#include <unistd.h>

FileDescriptor::FileDescriptor(const int file_descriptor) noexcept
    : file_descriptor_{file_descriptor} {}

FileDescriptor::~FileDescriptor() noexcept {
  if (file_descriptor_ == kInvalidFileDescriptor) {
    return;
  }

  close(file_descriptor_);
  file_descriptor_ = kInvalidFileDescriptor;
}

auto FileDescriptor::Get() const noexcept -> const int {
  return file_descriptor_;
}

auto FileDescriptor::IsValid() const noexcept -> bool {
  return file_descriptor_ != kInvalidFileDescriptor;
}

LinuxEngine::LinuxEngine(FileDescriptor &&epoll_fd) noexcept
    : epoll_fd_{std::move(epoll_fd)} {
  assert(epoll_fd_.IsValid() && "Invalid epoll file descriptor");
}

auto LinuxEngineFactory::Create() const noexcept -> Result<LinuxEngine> {
  auto fd = FileDescriptor{epoll_create1(0)};
  if (!fd.IsValid()) {
    return Error{ServerErrorCode::kEngineEpollCreationFailed};
  }

  return LinuxEngine{};
}

#endif // defined(__linux__)
