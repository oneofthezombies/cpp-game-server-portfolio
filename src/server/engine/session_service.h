#ifndef SERVER_ENGINE_SESSION_SERVICE_H
#define SERVER_ENGINE_SESSION_SERVICE_H

#include "core/core.h"

#include "common.h"
#include "event_loop.h"
#include "mail_center.h"
#include "protocol.h"

namespace engine {

class SessionService : public EventLoopHandler {
public:
protected:
  explicit SessionService(const std::string_view name) noexcept;

  virtual ~SessionService() noexcept = default;
  CLASS_KIND_MOVABLE(SessionService);

private:
};

using SessionServicePtr = std::unique_ptr<SessionService>;

} // namespace engine

#endif // SERVER_ENGINE_SESSION_SERVICE_H
