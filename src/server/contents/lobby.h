#ifndef SERVER_CONTENTS_LOBBY_H
#define SERVER_CONTENTS_LOBBY_H

#include "server/engine/socket_event_loop_handler.h"

#include "common.h"

namespace contents {

class Lobby final : public engine::SocketEventLoopHandler {
public:
  using Super = engine::SocketEventLoopHandler;

  explicit Lobby() noexcept = default;
  virtual ~Lobby() noexcept override = default;
  CLASS_KIND_MOVABLE(Lobby);

  [[nodiscard]] virtual auto
  OnInit(const engine::EventLoop &event_loop,
         const engine::Config &config) noexcept -> Result<Void> override;

  [[nodiscard]] virtual auto
  OnMail(const engine::EventLoop &event_loop,
         const engine::Mail &mail) noexcept -> Result<Void> override;

  [[nodiscard]] virtual auto OnSocketIn(
      const engine::EventLoop &event_loop,
      const engine::SocketId socket_id) noexcept -> Result<Void> override;

private:
  auto IsMatchable() const noexcept -> bool;
  auto NextBattleId() noexcept -> uint64_t;

  uint64_t next_battle_id_{};
};

} // namespace contents

#endif // SERVER_CONTENTS_LOBBY_H
