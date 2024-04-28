#include "event_loop_linux.h"

#include <sys/epoll.h>
#include <sys/socket.h>

#include <cstring>

#include "core/tiny_json.h"
#include "core/utils.h"
#include "core/utils_linux.h"
#include "file_descriptor_linux.h"
#include "mail_center.h"
#include "utils_linux.h"

using namespace engine;

auto
engine::EventLoopLinux::Builder::Build(
    std::string &&name,
    EventLoopHandlerPtr &&handler) const noexcept -> Result<EventLoopPtr> {
  using ResultT = Result<EventLoopPtr>;

  auto epoll_fd = FileDescriptorLinux{epoll_create1(0)};
  if (!epoll_fd.IsValid()) {
    return ResultT{
        Error::From(kEventLoopLinuxEpollCreate1Failed,
                    core::TinyJson{}
                        .Set("linux_error", core::LinuxError::FromErrno())
                        .IntoMap())};
  }

  auto mail_box_res = MailCenter::Global().Create(std::string{name});
  if (mail_box_res.IsErr()) {
    return ResultT{Error::From(mail_box_res.TakeErr())};
  }

  return ResultT{EventLoopPtr{new EventLoopLinux{std::move(mail_box_res.Ok()),
                                                 std::move(name),
                                                 std::move(handler),
                                                 std::move(epoll_fd)}}};
}

engine::EventLoopLinux::EventLoopLinux(MailBox &&mail_box,
                                       std::string &&name,
                                       EventLoopHandlerPtr &&handler,
                                       FileDescriptorLinux &&epoll_fd) noexcept
    : EventLoop{std::move(mail_box), std::move(name), std::move(handler)},
      epoll_fd_{std::move(epoll_fd)} {}

auto
engine::EventLoopLinux::Init(const Config &config) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = handler_->OnInit(*this, config); res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  return ResultT{Void{}};
}

auto
engine::EventLoopLinux::Add(const SocketId socket_id,
                            const EventLoopAddOptions &options) const noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  auto fd_res = FileDescriptorLinux::ParseSocketIdToFd(socket_id);
  if (fd_res.IsErr()) {
    return ResultT{Error::From(fd_res.TakeErr())};
  }

  const auto fd = fd_res.Ok();
  struct epoll_event ev {};
  ev.events = EventLoopAddOptionsToEpollEvents(options);
  ev.data.fd = fd;
  if (epoll_ctl(epoll_fd_.AsRaw(), EPOLL_CTL_ADD, fd, &ev) == -1) {
    return ResultT{
        Error::From(kEventLoopLinuxEpollCtlAddFailed,
                    core::TinyJson{}
                        .Set("linux_error", core::LinuxError::FromErrno())
                        .IntoMap())};
  }

  return ResultT{Void{}};
}

auto
engine::EventLoopLinux::Remove(const SocketId socket_id) const noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  auto fd_res = FileDescriptorLinux::ParseSocketIdToFd(socket_id);
  if (fd_res.IsErr()) {
    return ResultT{Error::From(fd_res.TakeErr())};
  }

  const auto fd = fd_res.Ok();
  if (epoll_ctl(epoll_fd_.AsRaw(), EPOLL_CTL_DEL, fd, nullptr) == -1) {
    return ResultT{
        Error::From(kEventLoopLinuxEpollCtlDeleteFailed,
                    core::TinyJson{}
                        .Set("linux_error", core::LinuxError::FromErrno())
                        .IntoMap())};
  }

  return ResultT{Void{}};
}

auto
engine::EventLoopLinux::Write(const SocketId socket_id,
                              const std::string_view data) const noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  auto fd_res = FileDescriptorLinux::ParseSocketIdToFd(socket_id);
  if (fd_res.IsErr()) {
    return ResultT{Error::From(fd_res.TakeErr())};
  }

  const auto fd = fd_res.Ok();
  const auto data_size = data.size();
  ssize_t written = 0;
  while (written < data_size) {
    const auto count = write(fd, data.data() + written, data_size - written);
    if (count == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        continue;
      }

      return ResultT{
          Error::From(kEventLoopLinuxWriteFailed,
                      core::TinyJson{}
                          .Set("linux_error", core::LinuxError::FromErrno())
                          .Set("fd", fd)
                          .IntoMap())};
    } else {
      written += count;

      if (count == 0) {
        return ResultT{Error::From(kEventLoopLinuxWriteClosed,
                                   core::TinyJson{}.Set("fd", fd).IntoMap())};
      }
    }
  }

  return ResultT{Void{}};
}

auto
engine::EventLoopLinux::Run() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  struct epoll_event events[kMaxEvents]{};
  auto shutdown{false};
  while (!shutdown) {
    auto mail = mail_box_.rx.TryReceive();
    if (mail) {
      if (auto shutdown_res = mail->body.Get("__shutdown");
          shutdown_res.IsOk()) {
        shutdown = true;
      }

      if (auto res = handler_->OnMail(*this, std::move(*mail)); res.IsErr()) {
        return ResultT{Error::From(res.TakeErr())};
      }
    }

    const auto fd_count = epoll_wait(epoll_fd_.AsRaw(), events, kMaxEvents, 0);
    if (fd_count == -1) {
      if (errno == EINTR) {
        continue;
      }

      return ResultT{
          Error::From(kEventLoopLinuxEpollWaitFailed,
                      core::TinyJson{}
                          .Set("linux_error", core::LinuxError::FromErrno())
                          .IntoMap())};
    }

    for (int i = 0; i < fd_count; ++i) {
      const auto &event = events[i];
      auto socket_id_res =
          FileDescriptorLinux::ParseFdToSocketId(event.data.fd);
      if (socket_id_res.IsErr()) {
        return ResultT{Error::From(socket_id_res.TakeErr())};
      }

      const auto socket_id = socket_id_res.Ok();
      if (event.events & EPOLLERR) {
        int code{};
        socklen_t code_size = sizeof(code);
        if (getsockopt(event.data.fd, SOL_SOCKET, SO_ERROR, &code, &code_size) <
            0) {
          return ResultT{
              Error::From(kEventLoopLinuxGetSocketOptionFailed,
                          core::TinyJson{}
                              .Set("linux_error", core::LinuxError::FromErrno())
                              .Set("fd", event.data.fd)
                              .Set("socket_id", socket_id)
                              .IntoMap())};
        }

        if (code == 0) {
          return ResultT{Error::From(kEventLoopLinuxSocketErrorZero,
                                     core::TinyJson{}
                                         .Set("fd", event.data.fd)
                                         .Set("socket_id", socket_id)
                                         .IntoMap())};
        }

        const auto description = std::string_view{strerror(code)};
        if (auto res =
                handler_->OnSocketError(*this, socket_id, code, description);
            res.IsErr()) {
          return ResultT{Error::From(res.TakeErr())};
        }
      }

      if (event.events & EPOLLHUP) {
        const auto fd = event.data.fd;
        core::Defer defer{[fd] {
          if (auto res = FileDescriptorLinux::Close(fd); res.IsErr()) {
            core::TinyJson{}
                .Set("message", "file descriptor close failed")
                .Set("error", res.Err())
                .LogLn();
          }
        }};

        if (auto res = handler_->OnSocketHangUp(*this, socket_id);
            res.IsErr()) {
          return ResultT{Error::From(res.TakeErr())};
        }
      }

      if (event.events & EPOLLIN) {
        if (auto res = handler_->OnSocketIn(*this, socket_id); res.IsErr()) {
          return ResultT{Error::From(res.TakeErr())};
        }
      }
    }
  }

  return ResultT{Void{}};
}
