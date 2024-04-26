#ifndef SERVER_ENGINE_SESSION_SERVICE_H
#define SERVER_ENGINE_SESSION_SERVICE_H

#include <unordered_set>

#include "core/core.h"

#include "common.h"
#include "mail_center.h"
#include "protocol.h"

namespace engine {

using SessionId = uint64_t;

class SessionService {
public:
  [[nodiscard]] auto Name() const noexcept -> std::string_view { return name_; }

  virtual auto OnSessionRegistered(const SessionId session_id,
                                   MailBody &&mail_body) noexcept -> void = 0;

  virtual auto OnSessionClientEventReceived(
      const SessionId session_id,
      core::MessageBody &&message_body) noexcept -> void = 0;

protected:
  explicit SessionService(const std::string_view name) noexcept : name_{name} {}

  virtual ~SessionService() noexcept = default;
  CLASS_KIND_MOVABLE(SessionService);

  [[nodiscard]] auto
  IsSessionRegistered(const SessionId session_id) const noexcept -> bool;

  [[nodiscard]] auto
  MoveSession(const SessionId session_id,
              const std::string_view to) noexcept -> Result<core::Void>;

  [[nodiscard]] auto
  SendServerEvent(const SessionId session_id,
                  core::MessageBody &&body) noexcept -> Result<core::Void>;

private:
  [[nodiscard]] auto
  RegisterSession(const SessionId session_id,
                  MailBody &&mail_body) noexcept -> Result<core::Void>;

  [[nodiscard]] auto
  UnregisterSession(const SessionId session_id) noexcept -> Result<MailBody>;

  std::unordered_set<SessionId> sessions_;
  std::string name_;

  friend class EventLoop;
};

using SessionServicePtr = std::unique_ptr<SessionService>;

} // namespace engine

#endif // SERVER_ENGINE_SESSION_SERVICE_H
