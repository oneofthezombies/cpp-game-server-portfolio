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
  [[nodiscard]] static auto
  GetKindId() noexcept -> ServiceKindId {
    return kServiceKindId_Match;
  }

  [[nodiscard]] static auto
  GetKindName() noexcept -> ServiceKindName {
    return "match";
  }

  explicit MatchService(const Pin<RunnerContext> runner_context) noexcept
      : SocketPoolService{runner_context, {kServiceKindId_Actor}} {
    RegisterEventHandler(EventSocketOpen::kEvent, OnSocketOpen);
    RegisterEventHandler(EventSocketClose::kEvent, OnSocketClose);
  }

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
  OnSocketOpen(MatchService* self,
               const FlatJson& data) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    auto socket_id_opt = data.TryGet<u64>(EventSocketOpen::kSocketId);
    if (!socket_id_opt) {
      return ResultT::Err(
          FlatJson{}
              .Set("message", "Failed to get socket id from data")
              .Take());
    }

    const auto socket_id = socket_id_opt.Unwrap();
    if (auto res = self->RegisterSocket(socket_id); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    auto stringified = FlatJsonStringifier{}.Stringify(
        FlatJson{}.Set("kind", "connect").Set("socket_id", socket_id).Take());
    if (stringified.IsErr()) {
      return ResultT::Err(stringified.TakeErr());
    }

    if (auto res = self->GetDependency<IoEventLoopService>()->WriteToFd(
            socket_id,
            stringified.TakeOk());
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    if (self->socket_ids_.size() >= 2) {
      const auto player1_it = self->socket_ids_.begin();
      const auto player1_socket_id = *player1_it;
      const auto player2_it = std::next(player1_it);
      const auto player2_socket_id = *player2_it;
      if (auto res = self->UnregisterSocket(player1_socket_id); res.IsErr()) {
        return ResultT::Err(res.TakeErr());
      }

      if (auto res = self->UnregisterSocket(player2_socket_id); res.IsErr()) {
        return ResultT::Err(res.TakeErr());
      }

      const auto battle_id = self->NextBattleId();
      self->GetDependency<ActorService>()->SendMail(
          "battle",
          EventBattleStart::kEvent,
          FlatJson{}
              .Set(EventBattleStart::kBattleId, battle_id)
              .Set(EventBattleStart::kPlayer1SocketId, player1_socket_id)
              .Set(EventBattleStart::kPlayer2SocketId, player2_socket_id)
              .Take());
    }

    return OkVoid();
  }

  [[nodiscard]] static auto
  OnSocketClose(MatchService* self,
                const FlatJson& data) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    auto socket_id = data.TryGet<u64>(EventSocketClose::kSocketId);
    if (!socket_id) {
      return ResultT::Err(
          FlatJson{}
              .Set("message", "Failed to get socket id from data")
              .Take());
    }

    if (auto res = self->UnregisterSocket(socket_id.Unwrap()); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    return OkVoid();
  }

  auto
  NextBattleId() noexcept -> u64 {
    return battle_id_++;
  }

  u64 battle_id_{};
};
