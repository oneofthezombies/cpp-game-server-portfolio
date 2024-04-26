#ifndef SERVER_EVENT_LOOP_LINUX_H
#define SERVER_EVENT_LOOP_LINUX_H

#include "core/core.h"

#include "file_descriptor_linux.h"
#include "mail_center.h"
#include "session_service.h"

class EventLoopLinux {
public:
  struct Context final {
    class Builder final {
    public:
      explicit Builder() noexcept = default;
      ~Builder() noexcept = default;
      CLASS_KIND_PINNABLE(Builder);

      [[nodiscard]] auto
      Build(const std::string_view name,
            SessionServicePtr &&session_service) const noexcept
          -> Result<Context>;
    };

    MailBox mail_box;
    std::string name;
    FileDescriptorLinux epoll_fd;
    SessionServicePtr session_service;

    explicit Context(MailBox &&mail_box, std::string &&name,
                     FileDescriptorLinux &&epoll_fd,
                     SessionServicePtr &&session_service) noexcept;
    ~Context() noexcept = default;
    CLASS_KIND_MOVABLE(Context);

    auto SendMail(std::string &&to, MailBody &&body) noexcept -> void;
  };

  virtual ~EventLoopLinux() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoopLinux);

  [[nodiscard]] auto Add(const FileDescriptorLinux::Raw fd,
                         uint32_t events) noexcept -> Result<Void>;

  [[nodiscard]] auto Delete(const FileDescriptorLinux::Raw fd) noexcept
      -> Result<Void>;

  [[nodiscard]] auto Write(const FileDescriptorLinux::Raw fd,
                           const std::string_view data) noexcept
      -> Result<Void>;

  [[nodiscard]] virtual auto Init(const std::string_view name,
                                  SessionServicePtr &&session_service) noexcept
      -> Result<Void>;

  [[nodiscard]] auto Run() noexcept -> Result<Void>;

  [[nodiscard]] virtual auto OnMailReceived(const Mail &mail) noexcept
      -> Result<Void> = 0;
  [[nodiscard]] virtual auto
  OnEpollEventReceived(const struct epoll_event &event) noexcept
      -> Result<Void> = 0;

protected:
  explicit EventLoopLinux() noexcept = default;

  void AssertInit() const noexcept;

  std::unique_ptr<Context> context_;

  static constexpr size_t kMaxEvents = 1024;
};

#endif // SERVER_EVENT_LOOP_LINUX_H
