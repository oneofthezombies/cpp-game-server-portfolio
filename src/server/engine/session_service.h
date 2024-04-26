#ifndef SERVER_ENGINE_SESSION_SERVICE_H
#define SERVER_ENGINE_SESSION_SERVICE_H

#include "core/core.h"

#include "common.h"
#include "mail_center.h"
#include "protocol.h"
#include "session.h"

namespace engine {

template <typename T = Void> class SessionService {
public:
  virtual auto OnSessionToRegister(const SessionId session_id,
                                   MailBody &&mail_body) noexcept -> T = 0;

  virtual auto OnSessionUnregistered(const SessionId session_id,
                                     T &&data) noexcept -> void = 0;

  virtual auto OnSessionClientEventReceived(const SessionId session_id,
                                            MessageBody &&message_body) noexcept
      -> void = 0;

protected:
  explicit SessionService() noexcept = default;
  virtual ~SessionService() noexcept = default;
  CLASS_KIND_MOVABLE(SessionService);

  [[nodiscard]] auto
  IsSessionRegistered(const SessionId session_id) const noexcept -> bool;

  [[nodiscard]] auto MoveSession(const SessionId session_id,
                                 const std::string_view to) noexcept
      -> Result<Void>;

  [[nodiscard]] auto SendServerEvent(const SessionId session_id,
                                     MessageBody &&body) noexcept
      -> Result<Void>;

private:
  [[nodiscard]] auto RegisterSession(const SessionId session_id,
                                     MailBody &&mail_body) noexcept
      -> Result<Void>;

  [[nodiscard]] auto UnregisterSession(const SessionId session_id) noexcept
      -> Result<MailBody>;

  std::unordered_map<SessionId, T> sessions_;

  friend class EventLoop;
};

} // namespace engine

#endif // SERVER_ENGINE_SESSION_SERVICE_H
