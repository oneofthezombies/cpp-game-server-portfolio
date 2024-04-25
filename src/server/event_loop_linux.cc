#include "event_loop_linux.h"

#include <cassert>

#include <sys/epoll.h>

#include "core/tiny_json.h"

#include "mail_center.h"
#include "utils_linux.h"

EventLoopLinux::Context::Context(MailBox &&mail_box, std::string &&name,
                                 FileDescriptorLinux &&epoll_fd) noexcept
    : mail_box{std::move(mail_box)}, name{std::move(name)},
      epoll_fd{std::move(epoll_fd)} {}

auto EventLoopLinux::Context::SendMail(std::string &&to,
                                       MailBody &&body) noexcept -> void {
  auto from = name;
  mail_box.tx.Send(Mail{std::move(from), std::move(to), std::move(body)});
}

auto EventLoopLinux::Context::Builder::Build(
    const std::string_view name) const noexcept -> Result<Context> {
  using ResultT = Result<Context>;

  auto epoll_fd = FileDescriptorLinux{epoll_create1(0)};
  if (!epoll_fd.IsValid()) {
    return ResultT{Error{
        Symbol::kEventLoopLinuxEpollCreate1Failed,
        TinyJson{}.Set("linux_error", LinuxError::FromErrno()).ToString()}};
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
    return ResultT{Error{
        Symbol::kEventLoopLinuxEpollCtlAddFailed,
        TinyJson{}.Set("linux_error", LinuxError::FromErrno()).ToString()}};
  }

  return ResultT{Void{}};
}

auto EventLoopLinux::Delete(const FileDescriptorLinux::Raw fd) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  AssertInit();
  if (epoll_ctl(context_->epoll_fd.AsRaw(), EPOLL_CTL_DEL, fd, nullptr) == -1) {
    return ResultT{Error{
        Symbol::kEventLoopLinuxEpollCtlDeleteFailed,
        TinyJson{}.Set("linux_error", LinuxError::FromErrno()).ToString()}};
  }

  return ResultT{Void{}};
}

auto EventLoopLinux::Write(const FileDescriptorLinux::Raw fd,
                           const std::string_view data) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  const auto data_size = data.size();
  const auto data_ptr = data.data();
  ssize_t written = 0;
  while (written < data_size) {
    const auto count = write(fd, data_ptr + written, data_size - written);
    if (count == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        continue;
      }

      return ResultT{Error{Symbol::kEventLoopLinuxWriteFailed,
                           TinyJson{}
                               .Set("linux_error", LinuxError::FromErrno())
                               .Set("fd", fd)
                               .ToString()}};
    } else {
      written += count;

      if (count == 0) {
        return ResultT{Error{Symbol::kEventLoopLinuxWriteClosed,
                             TinyJson{}.Set("fd", fd).ToString()}};
      }
    }
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

      return ResultT{Error{
          Symbol::kEventLoopLinuxEpollWaitFailed,
          TinyJson{}.Set("linux_error", LinuxError::FromErrno()).ToString()}};
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
