#ifndef SERVER_ENGINE_SESSION_EVENT_LOOP_HANDLER_H
#define SERVER_ENGINE_SESSION_EVENT_LOOP_HANDLER_H

#include "core/core.h"

#include "event_loop.h"

namespace engine {

class SessionEventLoopHandler : public EventLoopHandler {
protected:
  explicit SessionEventLoopHandler() noexcept = default;
  virtual ~SessionEventLoopHandler() noexcept = default;
  CLASS_KIND_MOVABLE(SessionEventLoopHandler);

private:
};

} // namespace engine

#endif // SERVER_ENGINE_SESSION_EVENT_LOOP_HANDLER_H
