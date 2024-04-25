#include "event_loop_linux.h"

#include <cassert>

#include <sys/epoll.h>

#include "utils_linux.h"

LinuxEventLoop::LinuxEventLoop(core::Rx<LinuxEventLoopEvent> &&event_loop_rx,
                               LinuxFileDescriptor &&epoll_fd) noexcept
    : event_loop_rx_{std::move(event_loop_rx)}, epoll_fd_{std::move(epoll_fd)} {
  assert(epoll_fd_.IsValid() && "epoll fd must be valid");
}

auto LinuxEventLoop::Add(const LinuxFileDescriptor::Raw fd,
                         uint32_t events) noexcept -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  struct epoll_event ev {};
  ev.events = events;
  ev.data.fd = fd;
  if (epoll_ctl(epoll_fd_.AsRaw(), EPOLL_CTL_ADD, fd, &ev) == -1) {
    return ResultT{Error{Symbol::kLinuxEventLoopEpollCtlAddFailed,
                         SB{}.Add(core::LinuxError::FromErrno()).Build()}};
  }

  return ResultT{core::Void{}};
}

auto LinuxEventLoop::Delete(const LinuxFileDescriptor::Raw fd,
                            uint32_t events) noexcept -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  struct epoll_event ev {};
  ev.events = events;
  ev.data.fd = fd;
  if (epoll_ctl(epoll_fd_.AsRaw(), EPOLL_CTL_DEL, fd, &ev) == -1) {
    return ResultT{Error{Symbol::kLinuxEventLoopEpollCtlDeleteFailed,
                         SB{}.Add(core::LinuxError::FromErrno()).Build()}};
  }

  return ResultT{core::Void{}};
}

auto LinuxEventLoop::Run(OnEventLoopEvent &&on_event_loop_event,
                         OnEpollEvent &&on_epoll_event) noexcept
    -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  struct epoll_event events[kMaxEvents]{};
  std::atomic<bool> shutdown{false};
  while (!shutdown) {
    auto event_loop_event = event_loop_rx_.TryReceive();
    if (event_loop_event) {
      if (event_loop_event->find("shutdown") != event_loop_event->end()) {
        shutdown = true;
      }

      if (auto res = on_event_loop_event(*event_loop_event); res.IsErr()) {
        return res;
      }
    }

    const auto fd_count = epoll_wait(epoll_fd_.AsRaw(), events, kMaxEvents, 0);
    if (fd_count == -1) {
      if (errno == EINTR) {
        continue;
      }

      return ResultT{Error{Symbol::kLinuxEventLoopEpollWaitFailed,
                           SB{}.Add(core::LinuxError::FromErrno()).Build()}};
    }

    for (int i = 0; i < fd_count; ++i) {
      const auto &event = events[i];
      if (auto res = on_epoll_event(event); res.IsErr()) {
        return res;
      }
    }
  }

  return ResultT{core::Void{}};
}

auto LinuxEventLoopBuilder::Build(core::Rx<LinuxEventLoopEvent> &&event_loop_rx)
    const noexcept -> Result<LinuxEventLoop> {
  using ResultT = Result<LinuxEventLoop>;

  auto epoll_fd = LinuxFileDescriptor{epoll_create1(0)};
  if (!epoll_fd.IsValid()) {
    return ResultT{Error{Symbol::kLinuxEventLoopEpollCreate1Failed,
                         SB{}.Add(core::LinuxError::FromErrno()).Build()}};
  }

  return ResultT{LinuxEventLoop{std::move(event_loop_rx), std::move(epoll_fd)}};
}
