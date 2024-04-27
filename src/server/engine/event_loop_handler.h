#ifndef SERVER_ENGINE_EVENT_LOOP_HANDLER_H
#define SERVER_ENGINE_EVENT_LOOP_HANDLER_H

#include <memory>

#include "core/core.h"

#include "common.h"
#include "session.h"

namespace engine {

struct Config;
class EventLoop;
struct EventLoopContext;

class EventLoopHandler {
public:
  explicit EventLoopHandler() noexcept;
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

} // namespace engine

#endif // SERVER_ENGINE_EVENT_LOOP_HANDLER_H
