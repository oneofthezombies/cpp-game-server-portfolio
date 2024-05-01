#ifndef SERVER_ENGINE_EVENT_LOOP_H
#define SERVER_ENGINE_EVENT_LOOP_H

#include "common.h"
#include "core/core.h"
#include "core/protocol.h"
#include "mail_center.h"

namespace engine {

struct Config;
class EventLoop;

class EventLoopHandler {
 public:
  explicit EventLoopHandler() noexcept = default;
  virtual ~EventLoopHandler() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoopHandler);

  [[nodiscard]] virtual auto
  OnInit(EventLoop &event_loop, const Config &config) noexcept
      -> Result<Void> = 0;

  [[nodiscard]] virtual auto
  OnMail(EventLoop &event_loop, const Mail &mail) noexcept -> Result<Void> = 0;

  [[nodiscard]] virtual auto
  OnSocketIn(EventLoop &event_loop, const SocketId socket_id) noexcept
      -> Result<Void> = 0;

  [[nodiscard]] virtual auto
  OnSocketHangUp(EventLoop &event_loop, const SocketId socket_id) noexcept
      -> Result<Void> = 0;

  [[nodiscard]] virtual auto
  OnSocketError(EventLoop &event_loop,
                const SocketId socket_id,
                const int code,
                const std::string_view description) noexcept
      -> Result<Void> = 0;
};

using EventLoopHandlerPtr = Owned<EventLoopHandler>;

struct EventLoopAddOptions {
  bool in{false};
  bool out{false};
  bool edge_trigger{false};
};

class EventLoop {
 public:
  explicit EventLoop(MailBox &&mail_box,
                     std::string &&name,
                     EventLoopHandlerPtr &&handler) noexcept;
  virtual ~EventLoop() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoop);

  [[nodiscard]] auto
  GetMailBox() const noexcept -> const MailBox &;
  [[nodiscard]] auto
  GetName() const noexcept -> std::string_view;

  auto
  SendMail(const std::string_view to, MailBody &&mail_body) const noexcept
      -> void;

  auto
  SendServerEvent(const SocketId socket_id, core::Message &&message) noexcept
      -> Result<Void>;

  [[nodiscard]] virtual auto
  Init(const Config &config) noexcept -> Result<Void> = 0;

  [[nodiscard]] virtual auto
  Run() noexcept -> Result<Void> = 0;

  [[nodiscard]] virtual auto
  Add(const SocketId socket_id,
      const EventLoopAddOptions &options) const noexcept -> Result<Void> = 0;

  [[nodiscard]] virtual auto
  Remove(const SocketId socket_id) const noexcept -> Result<Void> = 0;

  [[nodiscard]] virtual auto
  Write(const SocketId socket_id, const std::string_view data) const noexcept
      -> Result<Void> = 0;

  [[nodiscard]] virtual auto
  Read(const SocketId socket_id) const noexcept -> Result<std::string> = 0;

 protected:
  [[nodiscard]] auto
  NextMessageId() noexcept -> core::MessageId;

  MailBox mail_box_;
  std::string name_;
  EventLoopHandlerPtr handler_;
  core::MessageId next_message_id_{1};
};

using EventLoopPtr = Owned<EventLoop>;

}  // namespace engine

#endif  // SERVER_ENGINE_EVENT_LOOP_H
