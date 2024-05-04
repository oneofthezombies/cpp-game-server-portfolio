#include "common.h"
#include "kero/core/flat_json_parser.h"
#include "kero/core/utils.h"
#include "kero/engine/runner_context.h"
#include "kero/engine/service.h"
#include "kero/middleware/common.h"
#include "kero/middleware/io_event_loop_service.h"
#include "kero/middleware/socket_pool_service.h"

using namespace kero;

struct BattleState {
  u64 player1_socket_id;
  std::string player1_move;

  u64 player2_socket_id;
  std::string player2_move;
};

class BattleService final : public SocketPoolService<BattleService> {
 public:
  explicit BattleService(const Borrow<RunnerContext> runner_context) noexcept
      : SocketPoolService{runner_context, {}} {
    RegisterEventHandler(EventSocketOpen::kEvent, OnSocketOpen);
    RegisterEventHandler(EventSocketClose::kEvent, OnSocketClose);
    RegisterEventHandler(EventBattleStart::kEvent, OnBattleStart);
  }

  virtual ~BattleService() noexcept override = default;
  KERO_CLASS_KIND_MOVABLE(BattleService);
  KERO_SERVICE_KIND(kServiceKindId_Battle, "battle");

  auto
  OnCreate() noexcept -> Result<Void> override {
    using ResultT = Result<Void>;

    if (auto res = SocketPoolService::OnCreate(); res.IsErr()) {
      return res;
    }

    return OkVoid();
  }

  auto
  OnDestroy() noexcept -> void override {
    SocketPoolService::OnDestroy();
  }

 private:
  [[nodiscard]] static auto
  OnSocketOpen(BattleService* self,
               const FlatJson& data) noexcept -> Result<Void> {
    using ResultT = Result<Void>;
    return OkVoid();
  }

  [[nodiscard]] static auto
  OnSocketClose(BattleService* self,
                const FlatJson& data) noexcept -> Result<Void> {
    using ResultT = Result<Void>;
    return OkVoid();
  }

  [[nodiscard]] static auto
  OnBattleStart(BattleService* self,
                const FlatJson& data) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    auto battle_id_opt = data.TryGet<u64>(EventBattleStart::kBattleId);
    if (!battle_id_opt) {
      return ResultT::Err(
          FlatJson{}
              .Set("message", "Failed to get battle id from data")
              .Take());
    }

    auto player1_socket_id_opt =
        data.TryGet<u64>(EventBattleStart::kPlayer1SocketId);
    if (!player1_socket_id_opt) {
      return ResultT::Err(
          FlatJson{}
              .Set("message", "Failed to get player1 socket id from data")
              .Take());
    }

    auto player2_socket_id_opt =
        data.TryGet<u64>(EventBattleStart::kPlayer2SocketId);
    if (!player2_socket_id_opt) {
      return ResultT::Err(
          FlatJson{}
              .Set("message", "Failed to get player2 socket id from data")
              .Take());
    }

    const auto battle_id = battle_id_opt.Unwrap();
    const auto player1_socket_id = player1_socket_id_opt.Unwrap();
    const auto player2_socket_id = player2_socket_id_opt.Unwrap();

    if (auto res = self->RegisterSocket(player1_socket_id); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    if (auto res = self->RegisterSocket(player2_socket_id); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    self->battle_state_map_.emplace(battle_id,
                                    BattleState{
                                        .player1_socket_id = player1_socket_id,
                                        .player1_move = "",
                                        .player2_socket_id = player2_socket_id,
                                        .player2_move = "",
                                    });
    self->socket_to_battle_.emplace(player1_socket_id, battle_id);
    self->socket_to_battle_.emplace(player2_socket_id, battle_id);

    auto player1_stringified = FlatJsonStringifier{}.Stringify(
        FlatJson{}
            .Set("kind", "battle_start")
            .Set("battle_id", battle_id)
            .Set("my_socket_id", player1_socket_id)
            .Set("enemy_socket_id", player2_socket_id)
            .Take());
    if (player1_stringified.IsErr()) {
      return ResultT::Err(player1_stringified.TakeErr());
    }

    if (auto res = self->GetDependency<IoEventLoopService>()->WriteToFd(
            player1_socket_id,
            player1_stringified.TakeOk());
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    auto player2_stringified = FlatJsonStringifier{}.Stringify(
        FlatJson{}
            .Set("kind", "battle_start")
            .Set("battle_id", battle_id)
            .Set("my_socket_id", player2_socket_id)
            .Set("enemy_socket_id", player1_socket_id)
            .Take());
    if (player2_stringified.IsErr()) {
      return ResultT::Err(player2_stringified.TakeErr());
    }

    if (auto res = self->GetDependency<IoEventLoopService>()->WriteToFd(
            player2_socket_id,
            player2_stringified.TakeOk());
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    return OkVoid();
  }

  std::unordered_map<u64 /* battle_id */, BattleState> battle_state_map_;
  std::unordered_map<SocketId, u64 /* battle_id */> socket_to_battle_;
};
