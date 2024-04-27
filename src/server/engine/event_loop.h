#ifndef SERVER_ENGINE_EVENT_LOOP_H
#define SERVER_ENGINE_EVENT_LOOP_H

#include "core/core.h"

#include "common.h"
#include "mail_center.h"
#include "socket.h"

namespace engine {

struct Config;
class EventLoop;

class EventLoopHandler {
public:
  explicit EventLoopHandler() noexcept = default;
  virtual ~EventLoopHandler() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoopHandler);

  [[nodiscard]] virtual auto OnInit(const EventLoop &event_loop,
                                    const Config &config) noexcept
      -> Result<Void> = 0;

  [[nodiscard]] virtual auto OnMail(const EventLoop &event_loop,
                                    const Mail &mail) noexcept
      -> Result<Void> = 0;

  [[nodiscard]] virtual auto OnSocketIn(const EventLoop &event_loop,
                                        const SocketId socket_id) noexcept
      -> Result<Void> = 0;

  [[nodiscard]] virtual auto OnSocketHangUp(const EventLoop &event_loop,
                                            const SocketId socket_id) noexcept
      -> Result<Void> = 0;

  [[nodiscard]] virtual auto
  OnSocketError(const EventLoop &event_loop, const SocketId socket_id,
                const int code, const std::string_view description) noexcept
      -> Result<Void> = 0;
};

using EventLoopHandlerPtr = std::unique_ptr<EventLoopHandler>;

class EventLoop {
public:
  explicit EventLoop(MailBox &&mail_box, std::string &&name,
                     EventLoopHandlerPtr &&handler) noexcept;
  virtual ~EventLoop() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoop);

  [[nodiscard]] auto GetMailBox() const noexcept -> const MailBox &;
  [[nodiscard]] auto GetName() const noexcept -> std::string_view;

  auto SendMail(const std::string_view to, MailBody &&mail_body) const noexcept
      -> void;

  [[nodiscard]] virtual auto Init(const Config &config) noexcept
      -> Result<Void> = 0;
  [[nodiscard]] virtual auto Run() noexcept -> Result<Void> = 0;
  [[nodiscard]] virtual auto Add(const SocketId socket_id,
                                 const uint32_t events) const noexcept
      -> Result<Void> = 0;
  [[nodiscard]] virtual auto Delete(const SocketId socket_id) const noexcept
      -> Result<Void> = 0;
  [[nodiscard]] virtual auto Write(const SocketId socket_id,
                                   const std::string_view data) const noexcept
      -> Result<Void> = 0;

protected:
  MailBox mail_box_;
  std::string name_;
  EventLoopHandlerPtr handler_;
};

using EventLoopPtr = std::unique_ptr<EventLoop>;

} // namespace engine

#endif // SERVER_ENGINE_EVENT_LOOP_H
