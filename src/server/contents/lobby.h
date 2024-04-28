#ifndef SERVER_CONTENTS_LOBBY_H
#define SERVER_CONTENTS_LOBBY_H

#include "common.h"
#include "server/engine/socket_event_loop_handler.h"

namespace contents {

class Lobby final : public engine::SocketEventLoopHandler<Lobby> {
 public:
  using Super = engine::SocketEventLoopHandler<Lobby>;
  using Self = Lobby;

  explicit Lobby() noexcept = default;
  virtual ~Lobby() noexcept override = default;
  CLASS_KIND_MOVABLE(Lobby);

  [[nodiscard]] virtual auto
  OnInit(engine::EventLoop &event_loop,
         const engine::Config &config) noexcept -> Result<Void> override;

  [[nodiscard]] virtual auto
  OnMail(engine::EventLoop &event_loop,
         const engine::Mail &mail) noexcept -> Result<Void> override;

  [[nodiscard]] virtual auto
  OnSocketIn(engine::EventLoop &event_loop,
             const engine::SocketId socket_id) noexcept
      -> Result<Void> override;

 private:
  [[nodiscard]] auto
  IsMatchable() const noexcept -> bool;

  [[nodiscard]] auto
  NextBattleId() noexcept -> BattleId;

  [[nodiscard]] static auto
  OnConnect(Self &self,
            engine::EventLoop &event_loop,
            const engine::Mail &mail) noexcept -> Result<Void>;

  BattleId next_battle_id_{1};
};

}  // namespace contents

#endif  // SERVER_CONTENTS_LOBBY_H
