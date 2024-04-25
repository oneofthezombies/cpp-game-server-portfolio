#include "event_loop_linux.h"

#include <cassert>

#include <sys/epoll.h>

#include "utils_linux.h"

EventLoopLinux::EventLoopLinux(
    std::vector<Rx<EventLoopLinuxEvent>> &&event_loop_rxs,
    FileDescriptorLinux &&epoll_fd) noexcept
    : event_loop_rxs_{std::move(event_loop_rxs)},
      epoll_fd_{std::move(epoll_fd)} {
  assert(epoll_fd_.IsValid() && "epoll fd must be valid");
}

auto EventLoopLinux::Add(const FileDescriptorLinux::Raw fd,
                         uint32_t events) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  struct epoll_event ev {};
  ev.events = events;
  ev.data.fd = fd;
  if (epoll_ctl(epoll_fd_.AsRaw(), EPOLL_CTL_ADD, fd, &ev) == -1) {
    return ResultT{Error{Symbol::kEventLoopLinuxEpollCtlAddFailed,
                         SB{}.Add(LinuxError::FromErrno()).Build()}};
  }

  return ResultT{Void{}};
}

auto EventLoopLinux::Delete(const FileDescriptorLinux::Raw fd) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (epoll_ctl(epoll_fd_.AsRaw(), EPOLL_CTL_DEL, fd, nullptr) == -1) {
    return ResultT{Error{Symbol::kEventLoopLinuxEpollCtlDeleteFailed,
                         SB{}.Add(LinuxError::FromErrno()).Build()}};
  }

  return ResultT{Void{}};
}

auto EventLoopLinux::Run(OnEventLoopEvent &&on_event_loop_event,
                         OnEpollEvent &&on_epoll_event) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  struct epoll_event events[kMaxEvents]{};
  std::atomic<bool> shutdown{false};
  while (!shutdown) {
    for (const auto &event_loop_rx_ : event_loop_rxs_) {
      const auto event_loop_event = event_loop_rx_.TryReceive();
      if (event_loop_event) {
        if (event_loop_event->find("shutdown") != event_loop_event->end()) {
          shutdown = true;
        }

        if (auto res = on_event_loop_event(*event_loop_event); res.IsErr()) {
          return res;
        }
      }
    }

    const auto fd_count = epoll_wait(epoll_fd_.AsRaw(), events, kMaxEvents, 0);
    if (fd_count == -1) {
      if (errno == EINTR) {
        continue;
      }

      return ResultT{Error{Symbol::kEventLoopLinuxEpollWaitFailed,
                           SB{}.Add(LinuxError::FromErrno()).Build()}};
    }

    for (int i = 0; i < fd_count; ++i) {
      const auto &event = events[i];
      if (auto res = on_epoll_event(event); res.IsErr()) {
        return res;
      }
    }
  }

  return ResultT{Void{}};
}

auto EventLoopLinuxBuilder::Build(
    std::vector<Rx<EventLoopLinuxEvent>> &&event_loop_rxs) const noexcept
    -> Result<EventLoopLinux> {
  using ResultT = Result<EventLoopLinux>;

  auto epoll_fd = FileDescriptorLinux{epoll_create1(0)};
  if (!epoll_fd.IsValid()) {
    return ResultT{Error{Symbol::kEventLoopLinuxEpollCreate1Failed,
                         SB{}.Add(LinuxError::FromErrno()).Build()}};
  }

  return ResultT{
      EventLoopLinux{std::move(event_loop_rxs), std::move(epoll_fd)}};
}
