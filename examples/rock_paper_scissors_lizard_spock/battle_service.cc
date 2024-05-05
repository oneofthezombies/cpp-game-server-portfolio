#include "common.h"
#include "kero/core/flat_json.h"
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

static const std::unordered_map<std::string, std::string> kWinActionMap{
    {"scissors", "paper"},
    {"paper", "rock"},
    {"rock", "lizard"},
    {"lizard", "spock"},
    {"spock", "scissors"},
    {"scissors", "lizard"},
    {"lizard", "paper"},
    {"paper", "spock"},
    {"spock", "rock"},
    {"rock", "scissors"},
};

struct PlayerState {
  u64 battle_id{};
};

struct BattleState {
  SocketId player1_socket_id{};
  std::string player1_action;
  SocketId player2_socket_id{};
  std::string player2_action;
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

    if (auto res = RegisterMethodEventHandler(EventBattleAction::kEvent,
                                              &BattleService::OnBattleAction);
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
                                  .player1_action = "",
                                  .player2_socket_id = player2_socket_id,
                                  .player2_action = "",
                              });
    player_state_map_.emplace(player1_socket_id,
                              PlayerState{.battle_id = battle_id});
    player_state_map_.emplace(player2_socket_id,
                              PlayerState{.battle_id = battle_id});

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
  OnBattleAction(const FlatJson& data) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    const auto socket_id_opt = data.TryGet<u64>(EventBattleAction::kSocketId);
    if (!socket_id_opt) {
      return ResultT::Err(
          FlatJson{}
              .Set("message", "Failed to get socket id from data")
              .Take());
    }

    const auto socket_id = socket_id_opt.Unwrap();

    const auto action_opt =
        data.TryGet<std::string>(EventBattleAction::kAction);
    if (!action_opt) {
      return ResultT::Err(
          FlatJson{}.Set("message", "Failed to get action from data").Take());
    }

    const auto action = action_opt.Unwrap();

    const auto player_state_it = player_state_map_.find(socket_id);
    if (player_state_it == player_state_map_.end()) {
      return ResultT::Err(
          FlatJson{}.Set("message", "Failed to find player state").Take());
    }

    auto& player_state = player_state_it->second;
    const auto battle_state_it = battle_state_map_.find(player_state.battle_id);
    if (battle_state_it == battle_state_map_.end()) {
      return ResultT::Err(
          FlatJson{}.Set("message", "Failed to find battle state").Take());
    }

    auto& battle_state = battle_state_it->second;
    if (battle_state.player1_socket_id == socket_id) {
      battle_state.player1_action = action;
    } else if (battle_state.player2_socket_id == socket_id) {
      battle_state.player2_action = action;
    } else {
      return ResultT::Err(
          FlatJson{}.Set("message", "Failed to find player in battle").Take());
    }

    if (battle_state.player1_action.empty() ||
        battle_state.player2_action.empty()) {
      return OkVoid();
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
  std::unordered_map<SocketId, PlayerState> player_state_map_;
};
