#ifndef SERVER_ENGINE_SOCKET_EVENT_LOOP_HANDLER_H
#define SERVER_ENGINE_SOCKET_EVENT_LOOP_HANDLER_H

#include <unordered_set>

#include "core/core.h"

#include "event_loop.h"

namespace engine {

class SocketEventLoopHandler : public EventLoopHandler {
public:
  explicit SocketEventLoopHandler() noexcept;
  virtual ~SocketEventLoopHandler() noexcept = default;
  CLASS_KIND_MOVABLE(SocketEventLoopHandler);

  [[nodiscard]] virtual auto OnMail(const EventLoop &event_loop,
                                    const Mail &mail) noexcept
      -> Result<Void> override;

  [[nodiscard]] virtual auto OnSocketHangUp(const EventLoop &event_loop,
                                            const SocketId socket_id) noexcept
      -> Result<Void> override;

  [[nodiscard]] virtual auto
  OnSocketError(const EventLoop &event_loop, const SocketId socket_id,
                const int code, const std::string_view description) noexcept
      -> Result<Void> override;

  [[nodiscard]] auto RegisterSocket(const EventLoop &event_loop,
                                    const SocketId socket_id) noexcept
      -> Result<Void>;
  [[nodiscard]] auto UnregisterSocket(const EventLoop &event_loop,
                                      const SocketId socket_id) noexcept
      -> Result<Void>;

private:
  [[nodiscard]] auto AddSocketToSet(const SocketId socket_id) noexcept
      -> Result<Void>;
  [[nodiscard]] auto RemoveSocketFromSet(const SocketId socket_id) noexcept
      -> Result<Void>;

protected:
  std::unordered_set<SocketId> sockets_;
};

} // namespace engine

#endif // SERVER_ENGINE_SOCKET_EVENT_LOOP_HANDLER_H
