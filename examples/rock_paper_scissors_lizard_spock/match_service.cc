#include <unordered_map>

#include "common.h"
#include "kero/core/flat_json.h"
#include "kero/core/flat_json_parser.h"
#include "kero/core/utils.h"
#include "kero/engine/actor_service.h"
#include "kero/engine/common.h"
#include "kero/engine/runner_context.h"
#include "kero/middleware/common.h"
#include "kero/middleware/io_event_loop_service.h"
#include "kero/middleware/socket_pool_service.h"

using namespace kero;

class MatchService final : public SocketPoolService<MatchService> {
 public:
  explicit MatchService(const Borrow<RunnerContext> runner_context) noexcept
      : SocketPoolService{runner_context, {kServiceKindId_Actor}} {}

  virtual ~MatchService() noexcept override = default;
  KERO_CLASS_KIND_MOVABLE(MatchService);
  KERO_SERVICE_KIND(kServiceKindId_Match, "match");

  auto
  OnCreate() noexcept -> Result<Void> override {
    using ResultT = Result<Void>;

    if (auto res = SocketPoolService::OnCreate(); res.IsErr()) {
      return res;
    }

    if (auto res = RegisterMethodEventHandler(EventSocketMove::kEvent,
                                              &MatchService::OnSocketMove);
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    if (auto res = RegisterMethodEventHandler(EventSocketClose::kEvent,
                                              &MatchService::OnSocketClose);
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    if (auto res =
            RegisterMethodEventHandler(EventBattleSocketCount::kEvent,
                                       &MatchService::OnBattleSocketCount);
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    return OkVoid();
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
    if (auto res = RegisterSocket(socket_id); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    auto stringified = FlatJsonStringifier{}.Stringify(
        FlatJson{}.Set("event", "connect").Set("socket_id", socket_id).Take());
    if (stringified.IsErr()) {
      return ResultT::Err(stringified.TakeErr());
    }

    if (auto res = GetDependency<IoEventLoopService>()->WriteToFd(
            socket_id,
            stringified.TakeOk());
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    if (socket_map_.size() >= 2) {
      const auto player1_it = socket_map_.begin();
      const auto player1_socket_id = player1_it->first;
      const auto player2_it = std::next(player1_it);
      const auto player2_socket_id = player2_it->first;
      if (auto res = UnregisterSocket(player1_socket_id); res.IsErr()) {
        return ResultT::Err(res.TakeErr());
      }

      if (auto res = UnregisterSocket(player2_socket_id); res.IsErr()) {
        return ResultT::Err(res.TakeErr());
      }

      const auto min_battle_socket_count_it = std::min_element(
          battle_socket_count_map_.begin(),
          battle_socket_count_map_.end(),
          [](const auto& a, const auto& b) { return a.second < b.second; });
      if (min_battle_socket_count_it == battle_socket_count_map_.end()) {
        return ResultT::Err(
            FlatJson{}
                .Set("message", "Failed to get min battle socket count")
                .Take());
      }

      const auto& name = min_battle_socket_count_it->first;
      GetDependency<ActorService>()->SendMail(
          std::string{name},
          EventSocketMove::kEvent,
          FlatJson{}.Set(EventSocketMove::kSocketId, player1_socket_id).Take());
      GetDependency<ActorService>()->SendMail(
          std::string{name},
          EventSocketMove::kEvent,
          FlatJson{}.Set(EventSocketMove::kSocketId, player2_socket_id).Take());

      const auto battle_id = NextBattleId();
      GetDependency<ActorService>()->SendMail(
          std::string{name},
          EventBattleStart::kEvent,
          FlatJson{}
              .Set(EventBattleStart::kBattleId, battle_id)
              .Set(EventBattleStart::kPlayer1SocketId, player1_socket_id)
              .Set(EventBattleStart::kPlayer2SocketId, player2_socket_id)
              .Take());
    }

    return OkVoid();
  }

  [[nodiscard]] auto
  OnSocketClose(const FlatJson& data) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    auto socket_id = data.TryGet<u64>(EventSocketClose::kSocketId);
    if (!socket_id) {
      return ResultT::Err(
          FlatJson{}
              .Set("message", "Failed to get socket id from data")
              .Take());
    }

    if (auto res = UnregisterSocket(socket_id.Unwrap()); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    return OkVoid();
  }

  [[nodiscard]] auto
  OnBattleSocketCount(const FlatJson& data) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    const auto name_opt =
        data.TryGet<std::string>(EventBattleSocketCount::kName);
    if (!name_opt) {
      return ResultT::Err(
          FlatJson{}.Set("message", "Failed to get name from data").Take());
    }

    const auto count_opt = data.TryGet<u64>(EventBattleSocketCount::kCount);
    if (!count_opt) {
      return ResultT::Err(
          FlatJson{}.Set("message", "Failed to get count from data").Take());
    }

    const auto name = name_opt.Unwrap();
    const auto count = count_opt.Unwrap();
    battle_socket_count_map_[name] = count;

    return OkVoid();
  }

  auto
  NextBattleId() noexcept -> u64 {
    return battle_id_++;
  }

  u64 battle_id_{1};
  std::unordered_map<std::string /* name */, u64 /* count */>
      battle_socket_count_map_{};
};
