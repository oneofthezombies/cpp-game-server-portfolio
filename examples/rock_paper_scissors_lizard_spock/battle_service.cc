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

static const std::unordered_map<RpslsAction, std::vector<RpslsAction>> kWinMap{
    {RpslsAction::kRock, {RpslsAction::kScissors, RpslsAction::kLizard}},
    {RpslsAction::kPaper, {RpslsAction::kRock, RpslsAction::kSpock}},
    {RpslsAction::kScissors, {RpslsAction::kPaper, RpslsAction::kLizard}},
    {RpslsAction::kLizard, {RpslsAction::kSpock, RpslsAction::kPaper}},
    {RpslsAction::kSpock, {RpslsAction::kScissors, RpslsAction::kRock}},
};

struct RpslsResultInfo {
  RpslsResult player1;
  RpslsResult player2;
};

auto
GetResultInfo(const RpslsAction player1_action,
              const RpslsAction player2_action) noexcept -> RpslsResultInfo {
  if (player1_action == player2_action) {
    return RpslsResultInfo{
        .player1 = RpslsResult::kDraw,
        .player2 = RpslsResult::kDraw,
    };
  }

  const auto& wins = kWinMap.at(player1_action);
  if (std::find(wins.begin(), wins.end(), player2_action) != wins.end()) {
    return RpslsResultInfo{
        .player1 = RpslsResult::kWin,
        .player2 = RpslsResult::kLose,
    };
  }

  return RpslsResultInfo{
      .player1 = RpslsResult::kLose,
      .player2 = RpslsResult::kWin,
  };
}

struct PlayerState {
  u64 battle_id{};
};

struct BattleState {
  SocketId player1_socket_id{};
  RpslsAction player1_action;
  SocketId player2_socket_id{};
  RpslsAction player2_action;
  u32 remaining_socket_count{};
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
            .Set(EventBattleSocketCount::kCount, socket_map_.size())
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
                                  .player1_action = RpslsAction::kInvalid,
                                  .player2_socket_id = player2_socket_id,
                                  .player2_action = RpslsAction::kInvalid,
                                  .remaining_socket_count = 2,
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

    const auto action_opt = data.TryGet<std::underlying_type_t<RpslsAction>>(
        EventBattleAction::kAction);
    if (!action_opt) {
      return ResultT::Err(
          FlatJson{}.Set("message", "Failed to get action from data").Take());
    }

    const auto action = action_opt.Unwrap();
    const auto rpsls_action = RawToRpslsAction(action);
    if (rpsls_action == RpslsAction::kInvalid) {
      return ResultT::Err(FlatJson{}.Set("message", "Invalid action").Take());
    }

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
      battle_state.player1_action = rpsls_action;
    } else if (battle_state.player2_socket_id == socket_id) {
      battle_state.player2_action = rpsls_action;
    } else {
      return ResultT::Err(
          FlatJson{}.Set("message", "Failed to find player in battle").Take());
    }

    // If both players have made their actions
    if (battle_state.player1_action == RpslsAction::kInvalid ||
        battle_state.player2_action == RpslsAction::kInvalid) {
      return OkVoid();
    }

    const auto result_info =
        GetResultInfo(battle_state.player1_action, battle_state.player2_action);

    auto player1_stringified_res = FlatJsonStringifier{}.Stringify(
        FlatJson{}
            .Set("event", "battle_result")
            .Set("result",
                 static_cast<std::underlying_type_t<RpslsResult>>(
                     result_info.player1))
            .Take());
    if (player1_stringified_res.IsErr()) {
      return ResultT::Err(player1_stringified_res.TakeErr());
    }

    const auto player1_stringified = player1_stringified_res.TakeOk();
    if (auto res = GetDependency<IoEventLoopService>()->WriteToFd(
            battle_state.player1_socket_id,
            player1_stringified);
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    auto player2_stringified_res = FlatJsonStringifier{}.Stringify(
        FlatJson{}
            .Set("event", "battle_result")
            .Set("result",
                 static_cast<std::underlying_type_t<RpslsResult>>(
                     result_info.player2))
            .Take());
    if (player2_stringified_res.IsErr()) {
      return ResultT::Err(player2_stringified_res.TakeErr());
    }

    const auto player2_stringified = player2_stringified_res.TakeOk();
    if (auto res = GetDependency<IoEventLoopService>()->WriteToFd(
            battle_state.player2_socket_id,
            player2_stringified);
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
            .Set(EventBattleSocketCount::kCount, socket_map_.size())
            .Take());

    return OkVoid();
  }

  [[nodiscard]] auto
  UnregisterBattleSocket(const SocketId socket_id) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    if (auto res = UnregisterSocket(socket_id); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    const auto player_state_it = player_state_map_.find(socket_id);
    if (player_state_it != player_state_map_.end()) {
      const auto battle_id = player_state_it->second.battle_id;
      player_state_map_.erase(player_state_it);

      const auto battle_state_it = battle_state_map_.find(battle_id);
      if (battle_state_it != battle_state_map_.end()) {
        --battle_state_it->second.remaining_socket_count;
        if (battle_state_it->second.remaining_socket_count <= 0) {
          battle_state_map_.erase(battle_state_it);
          log::Debug("Battle ended").Data("battle_id", battle_id).Log();
        }
      }
    }

    const auto& name = GetDependency<ActorService>()->GetName();
    GetDependency<ActorService>()->SendMail(
        "match",
        EventBattleSocketCount::kEvent,
        FlatJson{}
            .Set(EventBattleSocketCount::kName, name)
            .Set(EventBattleSocketCount::kCount, socket_map_.size())
            .Take());

    return OkVoid();
  }

  std::unordered_map<u64 /* battle_id */, BattleState> battle_state_map_;
  std::unordered_map<SocketId, PlayerState> player_state_map_;
};
