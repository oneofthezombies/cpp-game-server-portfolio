#include "event_loop_linux.h"

#include <cassert>

#include <sys/epoll.h>

#include "mail_center.h"
#include "utils_linux.h"

EventLoopLinux::Context::Context(MailBox &&mail_box, std::string &&name,
                                 FileDescriptorLinux &&epoll_fd) noexcept
    : mail_box{std::move(mail_box)}, name{std::move(name)},
      epoll_fd{std::move(epoll_fd)} {}

auto EventLoopLinux::Context::Builder::Build(
    const std::string_view name) const noexcept -> Result<Context> {
  using ResultT = Result<Context>;

  auto epoll_fd = FileDescriptorLinux{epoll_create1(0)};
  if (!epoll_fd.IsValid()) {
    return ResultT{Error{Symbol::kEventLoopLinuxEpollCreate1Failed,
                         SB{}.Add(LinuxError::FromErrno()).Build()}};
  }

  auto mail_box_res = MailCenter::Global().Create(name);
  if (mail_box_res.IsErr()) {
    return ResultT{std::move(mail_box_res.Err())};
  }

  return ResultT{
      Context{std::move(mail_box_res.Ok()), std::string{name},
              std::move(epoll_fd)},
  };
}

auto EventLoopLinux::Init(const std::string_view name) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  assert(context_ == nullptr && "Must call Init() only once");

  auto context_res = Context::Builder{}.Build(name);
  if (context_res.IsErr()) {
    return ResultT{std::move(context_res.Err())};
  }

  context_ = std::make_unique<Context>(std::move(context_res.Ok()));

  AssertInit();
  return ResultT{Void{}};
}

auto EventLoopLinux::Add(const FileDescriptorLinux::Raw fd,
                         uint32_t events) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  AssertInit();
  struct epoll_event ev {};
  ev.events = events;
  ev.data.fd = fd;
  if (epoll_ctl(context_->epoll_fd.AsRaw(), EPOLL_CTL_ADD, fd, &ev) == -1) {
    return ResultT{Error{Symbol::kEventLoopLinuxEpollCtlAddFailed,
                         SB{}.Add(LinuxError::FromErrno()).Build()}};
  }

  return ResultT{Void{}};
}

auto EventLoopLinux::Delete(const FileDescriptorLinux::Raw fd) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  AssertInit();
  if (epoll_ctl(context_->epoll_fd.AsRaw(), EPOLL_CTL_DEL, fd, nullptr) == -1) {
    return ResultT{Error{Symbol::kEventLoopLinuxEpollCtlDeleteFailed,
                         SB{}.Add(LinuxError::FromErrno()).Build()}};
  }

  return ResultT{Void{}};
}

auto EventLoopLinux::Run() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  AssertInit();
  struct epoll_event events[kMaxEvents]{};
  std::atomic<bool> shutdown{false};
  while (!shutdown) {
    auto mail = context_->mail_box.rx.TryReceive();
    if (mail) {
      if (mail->body.find("shutdown") != mail->body.end()) {
        shutdown = true;
      }

      if (auto res = OnMailReceived(*mail); res.IsErr()) {
        return res;
      }
    }

    const auto fd_count =
        epoll_wait(context_->epoll_fd.AsRaw(), events, kMaxEvents, 0);
    if (fd_count == -1) {
      if (errno == EINTR) {
        continue;
      }

      return ResultT{Error{Symbol::kEventLoopLinuxEpollWaitFailed,
                           SB{}.Add(LinuxError::FromErrno()).Build()}};
    }

    for (int i = 0; i < fd_count; ++i) {
      const auto &event = events[i];
      if (auto res = OnEpollEventReceived(event); res.IsErr()) {
        return res;
      }
    }
  }

  return ResultT{Void{}};
}

auto EventLoopLinux::AssertInit() const noexcept -> void {
  assert(context_ != nullptr && "Must call Init() first");
}
