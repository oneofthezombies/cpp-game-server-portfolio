#include "io_event_loop_service.h"

#include <sys/epoll.h>
#include <sys/socket.h>

#include <cstring>

#include "kero/core/utils_linux.h"
#include "kero/engine/agent.h"
#include "kero/engine/constants.h"
#include "kero/log/log_builder.h"

using namespace kero;

namespace {

[[nodiscard]] static auto
AddOptionsToEpollEvents(
    const kero::IoEventLoopService::AddOptions options) noexcept -> uint32_t {
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

kero::IoEventLoopService::IoEventLoopService() noexcept
    : Service{ServiceKind::kIoEventLoop, {}} {}

auto
kero::IoEventLoopService::OnCreate(Agent& agent) noexcept -> Result<Void> {
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
kero::IoEventLoopService::OnDestroy(Agent& agent) noexcept -> void {
  if (!Fd::IsValid(epoll_fd_)) {
    return;
  }

  if (auto res = Fd::Close(epoll_fd_); res.IsErr()) {
    log::Error("Failed to close epoll fd").Data("fd", epoll_fd_).Log();
  }
}

auto
kero::IoEventLoopService::OnUpdate(Agent& agent) noexcept -> void {
  if (!Fd::IsValid(epoll_fd_)) {
    log::Error("Invalid epoll fd").Data("fd", epoll_fd_).Log();
    return;
  }

  struct epoll_event events[kMaxEvents]{};
  const auto fd_count = epoll_wait(epoll_fd_, events, kMaxEvents, 0);
  if (fd_count == -1) {
    if (errno == EINTR) {
      return;
    }

    log::Error("Failed to wait for epoll events")
        .Data("fd", epoll_fd_)
        .Data("errno", Errno::FromErrno())
        .Log();
    return;
  }

  for (int i = 0; i < fd_count; ++i) {
    const auto& event = events[i];
    if (auto res = OnUpdateEpollEvent(agent, event); res.IsErr()) {
      log::Error("Failed to update epoll event")
          .Data("fd", event.data.fd)
          .Data("error", res.TakeErr())
          .Log();
      continue;
    }
  }
}

auto
kero::IoEventLoopService::OnUpdateEpollEvent(
    Agent& agent, const struct epoll_event& event) noexcept -> Result<Void> {
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
    agent.Invoke(
        EventSocketError::kEvent,
        Dict{}
            .Set(EventSocketError::kFd, static_cast<int64_t>(event.data.fd))
            .Set(EventSocketError::kErrorCode, static_cast<int64_t>(code))
            .Set(EventSocketError::kErrorDescription,
                 std::string{description}));
  }

  if (event.events & EPOLLHUP) {
    agent.Invoke(
        EventSocketClose::kEvent,
        Dict{}.Set(EventSocketClose::kFd, static_cast<int64_t>(event.data.fd)));

    if (auto res = Fd::Close(event.data.fd); res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }
  }

  if (event.events & EPOLLIN) {
    agent.Invoke(
        EventSocketRead::kEvent,
        Dict{}.Set(EventSocketRead::kFd, static_cast<int64_t>(event.data.fd)));
  }

  return ResultT::Ok(Void{});
}

auto
kero::IoEventLoopService::AddFd(const Fd::Value fd,
                                const AddOptions options) const noexcept
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

auto
kero::IoEventLoopService::RemoveFd(const Fd::Value fd) const noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (!Fd::IsValid(epoll_fd_)) {
    return ResultT::Err(Error::From(kInvalidEpollFd));
  }

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    return ResultT::Err(Error::From(
        Errno::FromErrno()
            .IntoDict()
            .Set("message", std::string{"Failed to remove fd from epoll"})
            .Set("fd", static_cast<int64_t>(fd))
            .Take()));
  }

  return ResultT::Ok(Void{});
}

auto
kero::IoEventLoopService::WriteToFd(const Fd::Value fd,
                                    const std::string_view data) const noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  const auto data_size = data.size();
  const auto data_ptr = data.data();
  auto data_sent = 0;
  while (data_sent < data_size) {
    const auto sent = send(fd, data_ptr + data_sent, data_size - data_sent, 0);
    if (sent == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        continue;
      }

      return ResultT::Err(Error::From(
          Errno::FromErrno()
              .IntoDict()
              .Set("message", std::string{"Failed to send data to fd"})
              .Set("fd", static_cast<int64_t>(fd))
              .Set("data", std::string{data_ptr, data_size})
              .Take()));
    }

    data_sent += sent;
  }

  return ResultT::Ok(Void{});
}

auto
kero::IoEventLoopService::ReadFromFd(Agent& agent,
                                     const Fd::Value fd) const noexcept
    -> Result<std::string> {
  using ResultT = Result<std::string>;

  std::string buffer(4096, '\0');
  size_t total_read{0};
  while (true) {
    const auto read = recv(fd, buffer.data(), buffer.size(), 0);
    if (read == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        log::Error("Failed to read data from fd")
            .Data("fd", fd)
            .Data("errno", Errno::FromErrno())
            .Log();
        continue;
      }

      return ResultT::Err(Error::From(
          Errno::FromErrno()
              .IntoDict()
              .Set("message", std::string{"Failed to read data from fd"})
              .Set("fd", static_cast<int64_t>(fd))
              .Take()));
    }

    if (read == 0) {
      agent.Invoke(EventSocketClose::kEvent,
                   Dict{}.Set(EventSocketClose::kFd, static_cast<int64_t>(fd)));
      return ResultT::Err(
          Error::From(kSocketClosed,
                      Dict{}.Set("fd", static_cast<int64_t>(fd)).Take()));
    }

    total_read += read;
  }

  return ResultT::Ok(std::string{buffer.data(), total_read});
}
