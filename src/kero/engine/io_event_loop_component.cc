#include "io_event_loop_component.h"

#include <sys/epoll.h>
#include <sys/socket.h>

#include <cstring>

#include "kero/engine/constants.h"
#include "kero/engine/utils_linux.h"

using namespace kero;

namespace {

[[nodiscard]] static auto
AddOptionsToEpollEvents(
    const kero::IoEventLoopComponent::AddOptions options) noexcept -> uint32_t {
  uint32_t events{0};

  if (options.in) {
    events |= EPOLLIN;
  }

  if (options.out) {
    events |= EPOLLOUT;
  }

  if (options.edge_trigger) {
    events |= EPOLLET;
  }

  return events;
}

}  // namespace

kero::IoEventLoopComponent::IoEventLoopComponent() noexcept
    : Component{ComponentKind::kIoEventLoop} {}

auto
kero::IoEventLoopComponent::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  const auto epoll_fd = epoll_create1(0);
  if (!Fd::IsValid(epoll_fd)) {
    return ResultT::Err(
        Error::From(Errno::FromErrno()
                        .IntoDict()
                        .Set("message", std::string{"Failed to create epoll"})
                        .Take()));
  }

  epoll_fd_ = epoll_fd;
  return ResultT::Ok(Void{});
}

auto
kero::IoEventLoopComponent::OnDestroy(Agent& agent) noexcept -> void {
  if (!Fd::IsValid(epoll_fd_)) {
    return;
  }

  if (auto res = Fd::Close(epoll_fd_); res.IsErr()) {
    // TODO: log error
  }
}

auto
kero::IoEventLoopComponent::OnUpdate(Agent& agent) noexcept -> void {
  if (!Fd::IsValid(epoll_fd_)) {
    // TODO: log error
    return;
  }

  struct epoll_event events[kMaxEvents]{};
  const auto fd_count = epoll_wait(epoll_fd_, events, kMaxEvents, 0);
  if (fd_count == -1) {
    if (errno == EINTR) {
      return;
    }

    // TODO: log error
    return;
  }

  for (int i = 0; i < fd_count; ++i) {
    const auto& event = events[i];
    if (auto res = OnUpdateEpollEvent(event); res.IsErr()) {
      // TODO: log error
      continue;
    }
  }
}

auto
kero::IoEventLoopComponent::OnUpdateEpollEvent(
    const struct epoll_event& event) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (event.events & EPOLLERR) {
    int code{};
    socklen_t code_size = sizeof(code);
    if (getsockopt(event.data.fd, SOL_SOCKET, SO_ERROR, &code, &code_size) <
        0) {
      return ResultT::Err(Error::From(
          Errno::FromErrno()
              .IntoDict()
              .Set("message", std::string{"Failed to get socket error"})
              .Set("fd", static_cast<int64_t>(event.data.fd))
              .Take()));
    }

    if (code == 0) {
      return ResultT::Err(
          Error::From(Dict{}
                          .Set("message", std::string{"Socket error is zero"})
                          .Set("fd", static_cast<int64_t>(event.data.fd))
                          .Take()));
    }

    const auto description = std::string_view{strerror(code)};
    // TODO: propagate socket error event
  }

  if (event.events & EPOLLHUP) {
    // TODO: propagate socket close event

    if (auto res = Fd::Close(event.data.fd); res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }
  }

  if (event.events & EPOLLIN) {
    // TODO: propagate socket read event
  }

  return ResultT::Ok(Void{});
}

auto
kero::IoEventLoopComponent::AddFd(const Fd::Value fd,
                                  const AddOptions options) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (!Fd::IsValid(epoll_fd_)) {
    return ResultT::Err(Error::From(kInvalidEpollFd));
  }

  struct epoll_event ev {};
  ev.events = AddOptionsToEpollEvents(options);
  ev.data.fd = fd;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
    return ResultT::Err(Error::From(
        Errno::FromErrno()
            .IntoDict()
            .Set("message", std::string{"Failed to add fd to epoll"})
            .Set("fd", static_cast<int64_t>(fd))
            .Take()));
  }

  return ResultT::Ok(Void{});
}
