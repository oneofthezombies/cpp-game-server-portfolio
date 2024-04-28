#ifndef SERVER_CONTENTS_BATTLE_H
#define SERVER_CONTENTS_BATTLE_H

#include "common.h"
#include "core/core.h"
#include "server/engine/socket_event_loop_handler.h"

namespace contents {

struct BattleState {
  SocketId first_socket_id;
  std::string first_socket_move;

  SocketId second_socket_id;
  std::string second_socket_move;

  explicit BattleState() noexcept = default;
  ~BattleState() noexcept = default;
  CLASS_KIND_MOVABLE(BattleState);
};

class Battle final : public engine::SocketEventLoopHandler<Battle> {
 public:
  using Super = engine::SocketEventLoopHandler<Battle>;
  using Self = Battle;

  explicit Battle() noexcept = default;
  virtual ~Battle() noexcept override = default;
  CLASS_KIND_MOVABLE(Battle);

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
  [[nodiscard]] static auto
  OnStart(Self &self,
          engine::EventLoop &event_loop,
          const engine::Mail &mail) noexcept -> Result<Void>;

  [[nodiscard]] static auto
  OnBattleMove(Self &self,
               engine::EventLoop &event_loop,
               const engine::SocketId socket_id,
               core::Message &&message) noexcept -> Result<Void>;

  std::unordered_map<BattleId, BattleState> battle_states_;
  std::unordered_map<SocketId, BattleId> socket_id_to_battle_ids_;
};

}  // namespace contents

#endif  // SERVER_CONTENTS_BATTLE_H
