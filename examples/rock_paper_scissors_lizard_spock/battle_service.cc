#include "common.h"
#include "kero/core/flat_json_parser.h"
#include "kero/core/utils.h"
#include "kero/engine/actor_service.h"
#include "kero/engine/common.h"
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
      : SocketPoolService{runner_context, {kServiceKindId_Actor}} {}

  virtual ~BattleService() noexcept override = default;
  KERO_CLASS_KIND_MOVABLE(BattleService);
  KERO_SERVICE_KIND(kServiceKindId_Battle, "battle");

  auto
  OnCreate() noexcept -> Result<Void> override {
    using ResultT = Result<Void>;

    if (auto res = SocketPoolService::OnCreate(); res.IsErr()) {
      return res;
    }

    if (auto res = RegisterMethodEventHandler(EventSocketMove::kEvent,
                                              &BattleService::OnSocketMove);
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    if (auto res = RegisterMethodEventHandler(EventSocketClose::kEvent,
                                              &BattleService::OnSocketClose);
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    if (auto res = RegisterMethodEventHandler(EventBattleStart::kEvent,
                                              &BattleService::OnBattleStart);
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    const auto& name = GetDependency<ActorService>()->GetName();
    GetDependency<ActorService>()->SendMail(
        "match",
        EventBattleSocketCount::kEvent,
        FlatJson{}
            .Set(EventBattleSocketCount::kName, name)
            .Set(EventBattleSocketCount::kCount, socket_ids_.size())
            .Take());

    return OkVoid();
  }

  auto
  OnDestroy() noexcept -> void override {
    SocketPoolService::OnDestroy();
  }

 private:
  [[nodiscard]] auto
  OnSocketMove(const FlatJson& data) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    auto socket_id_opt = data.TryGet<u64>(EventSocketMove::kSocketId);
    if (!socket_id_opt) {
      return ResultT::Err(
          FlatJson{}
              .Set("message", "Failed to get socket id from data")
              .Take());
    }

    const auto socket_id = socket_id_opt.Unwrap();
    if (auto res = RegisterBattleSocket(socket_id); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    return OkVoid();
  }

  [[nodiscard]] auto
  OnSocketClose(const FlatJson& data) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    auto socket_id_opt = data.TryGet<u64>(EventSocketClose::kSocketId);
    if (!socket_id_opt) {
      return ResultT::Err(
          FlatJson{}
              .Set("message", "Failed to get socket id from data")
              .Take());
    }

    const auto socket_id = socket_id_opt.Unwrap();
    if (auto res = UnregisterBattleSocket(socket_id); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    return OkVoid();
  }

  [[nodiscard]] auto
  OnBattleStart(const FlatJson& data) noexcept -> Result<Void> {
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

    battle_state_map_.emplace(battle_id,
                              BattleState{
                                  .player1_socket_id = player1_socket_id,
                                  .player1_move = "",
                                  .player2_socket_id = player2_socket_id,
                                  .player2_move = "",
                              });
    socket_to_battle_.emplace(player1_socket_id, battle_id);
    socket_to_battle_.emplace(player2_socket_id, battle_id);

    auto player1_stringified_res = FlatJsonStringifier{}.Stringify(
        FlatJson{}
            .Set("event", "battle_start")
            .Set("battle_id", battle_id)
            .Set("opponent_socket_id", player2_socket_id)
            .Take());
    if (player1_stringified_res.IsErr()) {
      return ResultT::Err(player1_stringified_res.TakeErr());
    }

    const auto player1_stringified = player1_stringified_res.TakeOk();
    if (auto res =
            GetDependency<IoEventLoopService>()->WriteToFd(player1_socket_id,
                                                           player1_stringified);
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    auto player2_stringified = FlatJsonStringifier{}.Stringify(
        FlatJson{}
            .Set("event", "battle_start")
            .Set("battle_id", battle_id)
            .Set("opponent_socket_id", player1_socket_id)
            .Take());
    if (player2_stringified.IsErr()) {
      return ResultT::Err(player2_stringified.TakeErr());
    }

    if (auto res = GetDependency<IoEventLoopService>()->WriteToFd(
            player2_socket_id,
            player2_stringified.TakeOk());
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    return OkVoid();
  }

  [[nodiscard]] auto
  RegisterBattleSocket(const SocketId socket_id) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    if (auto res = RegisterSocket(socket_id); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    const auto& name = GetDependency<ActorService>()->GetName();
    GetDependency<ActorService>()->SendMail(
        "match",
        EventBattleSocketCount::kEvent,
        FlatJson{}
            .Set(EventBattleSocketCount::kName, name)
            .Set(EventBattleSocketCount::kCount, socket_ids_.size())
            .Take());

    return OkVoid();
  }

  [[nodiscard]] auto
  UnregisterBattleSocket(const SocketId socket_id) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    if (auto res = UnregisterSocket(socket_id); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    const auto& name = GetDependency<ActorService>()->GetName();
    GetDependency<ActorService>()->SendMail(
        "match",
        EventBattleSocketCount::kEvent,
        FlatJson{}
            .Set(EventBattleSocketCount::kName, name)
            .Set(EventBattleSocketCount::kCount, socket_ids_.size())
            .Take());

    return OkVoid();
  }

  std::unordered_map<u64 /* battle_id */, BattleState> battle_state_map_;
  std::unordered_map<SocketId, u64 /* battle_id */> socket_to_battle_;
};
