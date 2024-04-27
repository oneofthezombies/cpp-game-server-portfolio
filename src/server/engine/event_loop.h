#ifndef SERVER_ENGINE_EVENT_LOOP_H
#define SERVER_ENGINE_EVENT_LOOP_H

#include "core/core.h"

#include "common.h"
#include "mail_center.h"
#include "session.h"

namespace engine {

struct Config;
class EventLoop;

struct EventLoopContext {
  MailBox mail_box;
  std::string name;

  explicit EventLoopContext(MailBox &&mail_box, std::string &&name) noexcept;
  ~EventLoopContext() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoopContext);
};

class EventLoopHandler {
public:
  explicit EventLoopHandler() noexcept = default;
  virtual ~EventLoopHandler() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoopHandler);

  [[nodiscard]] virtual auto OnInit(const Config &config,
                                    const EventLoop &event_loop) noexcept
      -> Result<Void> = 0;

  [[nodiscard]] virtual auto OnSessionEvent(const EventLoopContext &context,
                                            const SessionId session_id,
                                            const uint32_t events) noexcept
      -> Result<Void> = 0;
};

using EventLoopHandlerPtr = std::unique_ptr<EventLoopHandler>;

class EventLoop {
public:
  explicit EventLoop(EventLoopContext &&context,
                     EventLoopHandlerPtr &&handler) noexcept;
  virtual ~EventLoop() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoop);

  [[nodiscard]] virtual auto Init(const Config &config) noexcept
      -> Result<Void> = 0;
  [[nodiscard]] virtual auto Run() noexcept -> Result<Void> = 0;
  [[nodiscard]] virtual auto Name() const noexcept -> std::string_view = 0;

  [[nodiscard]] virtual auto Add(const SessionId session_id,
                                 const uint32_t events) const noexcept
      -> Result<Void> = 0;

protected:
  EventLoopContext context_;
  EventLoopHandlerPtr handler_;
};

using EventLoopPtr = std::unique_ptr<EventLoop>;

} // namespace engine

#endif // SERVER_ENGINE_EVENT_LOOP_H
