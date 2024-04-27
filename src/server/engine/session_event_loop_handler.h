#ifndef SERVER_ENGINE_SESSION_EVENT_LOOP_HANDLER_H
#define SERVER_ENGINE_SESSION_EVENT_LOOP_HANDLER_H

#include <unordered_set>

#include "core/core.h"

#include "event_loop.h"

namespace engine {

class SessionEventLoopHandler : public EventLoopHandler {
protected:
  explicit SessionEventLoopHandler() noexcept = default;
  virtual ~SessionEventLoopHandler() noexcept = default;
  CLASS_KIND_MOVABLE(SessionEventLoopHandler);

  [[nodiscard]] virtual auto OnInit(const Config &config,
                                    const EventLoop &event_loop) noexcept
      -> Result<Void> override;

  [[nodiscard]] virtual auto OnMail(const EventLoopContext &context,
                                    Mail &&mail) noexcept
      -> Result<Void> override;

  [[nodiscard]] virtual auto OnSessionEvent(const EventLoopContext &context,
                                            const SessionId session_id,
                                            const uint32_t events) noexcept
      -> Result<Void> override;

private:
  std::unordered_set<SessionId> sessions_;
};

} // namespace engine

#endif // SERVER_ENGINE_SESSION_EVENT_LOOP_HANDLER_H
