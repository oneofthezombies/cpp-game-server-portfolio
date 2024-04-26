#ifndef SERVER_ENGINE_EVENT_LOOP_HANDLER_H
#define SERVER_ENGINE_EVENT_LOOP_HANDLER_H

#include <memory>

#include "core/core.h"

#include "common.h"

namespace engine {

struct Config;
class EventLoop;

class EventLoopHandler {
public:
  virtual ~EventLoopHandler() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoopHandler);

  [[nodiscard]] virtual auto OnInit(const Config &config,
                                    const EventLoop &event_loop) noexcept
      -> Result<Void> = 0;
};

using EventLoopHandlerPtr = std::unique_ptr<EventLoopHandler>;

} // namespace engine

#endif // SERVER_ENGINE_EVENT_LOOP_HANDLER_H
