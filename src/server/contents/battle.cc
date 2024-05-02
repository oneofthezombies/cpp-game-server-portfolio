#include "battle.h"

#include <string_view>
#include <unordered_map>
#include <vector>

#include "core/tiny_json.h"

using namespace contents;

namespace {

static const std::unordered_map<std::string_view, std::vector<std::string_view>>
    kRules = {{"rock", {"scissors", "lizard"}},
              {"paper", {"rock", "spock"}},
              {"scissors", {"paper", "lizard"}},
              {"lizard", {"spock", "paper"}},
              {"spock", {"scissors", "rock"}}};

}  // namespace

auto
contents::Battle::OnInit(engine::EventLoop &event_loop,
                         const engine::Config &config) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Super::OnInit(event_loop, config); res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  mail_handlers_.emplace("start", OnStart);
  message_handlers_.emplace("battle_move", OnBattleMove);

  return ResultT{Void{}};
}

auto
contents::Battle::OnMail(engine::EventLoop &event_loop,
                         const engine::Mail &mail) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Super::OnMail(event_loop, mail); res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  return ResultT{Void{}};
}

auto
contents::Battle::OnSocketIn(engine::EventLoop &event_loop,
                             const engine::SocketId socket_id) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Super::OnSocketIn(event_loop, socket_id); res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  return ResultT{Void{}};
}

auto
contents::Battle::OnStart(Self &self,
                          engine::EventLoop &event_loop,
                          const engine::Mail &mail) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  auto battle_id_res = mail.body.GetAsNumber<BattleId>("battle_id");
  if (battle_id_res.IsErr()) {
    return ResultT{Error::From(battle_id_res.TakeErr())};
  }

  const auto battle_id = battle_id_res.TakeOk();

  auto first_socket_id_res = mail.body.GetAsNumber<SocketId>("first_socket_id");
  if (first_socket_id_res.IsErr()) {
    return ResultT{Error::From(first_socket_id_res.TakeErr())};
  }

  const auto first_socket_id = first_socket_id_res.TakeOk();
  if (auto res = self.RegisterSocket(event_loop, first_socket_id);
      res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  self.socket_id_to_battle_ids_.emplace(first_socket_id, battle_id);

  auto second_socket_id_res =
      mail.body.GetAsNumber<SocketId>("second_socket_id");
  if (second_socket_id_res.IsErr()) {
    return ResultT{Error::From(second_socket_id_res.TakeErr())};
  }

  const auto second_socket_id = second_socket_id_res.TakeOk();
  if (auto res = self.RegisterSocket(event_loop, second_socket_id);
      res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  self.socket_id_to_battle_ids_.emplace(second_socket_id, battle_id);

  BattleState battle_state;
  battle_state.first_socket_id = first_socket_id;
  battle_state.second_socket_id = second_socket_id;
  self.battle_states_.emplace(battle_id, std::move(battle_state));

  if (auto res = event_loop.SendServerEvent(
          first_socket_id,
          core::JsonParser{}
              .Set("kind", "battle_start")
              .Set("battle_id", battle_id)
              .Set("opponent_socket_id", second_socket_id)
              .Take());
      res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  if (auto res = event_loop.SendServerEvent(
          second_socket_id,
          core::JsonParser{}
              .Set("kind", "battle_start")
              .Set("battle_id", battle_id)
              .Set("opponent_socket_id", first_socket_id)
              .Take());
      res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  return ResultT{Void{}};
}

auto
contents::Battle::OnBattleMove(Self &self,
                               engine::EventLoop &event_loop,
                               const engine::SocketId socket_id,
                               core::Message &&message) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  // find state
  auto battle_id_res = self.socket_id_to_battle_ids_.find(socket_id);
  if (battle_id_res == self.socket_id_to_battle_ids_.end()) {
    return ResultT{
        Error::From(kBattleBattleIdNotFound,
                    core::JsonParser{}.Set("socket_id", socket_id).IntoMap())};
  }

  const auto battle_id = battle_id_res->second;
  auto battle_state_res = self.battle_states_.find(battle_id);
  if (battle_state_res == self.battle_states_.end()) {
    return ResultT{
        Error::From(kBattleBattleStateNotFound,
                    core::JsonParser{}.Set("battle_id", battle_id).IntoMap())};
  }

  auto move_res = message.Get("move");
  if (move_res.IsErr()) {
    return ResultT{Error::From(move_res.TakeErr())};
  }

  auto move = move_res.TakeOk();

  // check move
  if (kRules.find(move) == kRules.end()) {
    return ResultT{Error::From(kBattleMoveNotFound,
                               core::JsonParser{}.Set("move", move).IntoMap())};
  }

  auto &battle_state = battle_state_res->second;
  if (battle_state.first_socket_id == socket_id) {
    if (!battle_state.first_socket_move.empty()) {
      return ResultT{Error::From(kBattleSocketMoveAlreadySet,
                                 core::JsonParser{}
                                     .Set("socket_id", socket_id)
                                     .Set("battle_id", battle_id)
                                     .IntoMap())};
    }

    battle_state.first_socket_move = move_res.TakeOk();
  } else if (battle_state.second_socket_id == socket_id) {
    if (!battle_state.second_socket_move.empty()) {
      return ResultT{Error::From(kBattleSocketMoveAlreadySet,
                                 core::JsonParser{}
                                     .Set("socket_id", socket_id)
                                     .Set("battle_id", battle_id)
                                     .IntoMap())};
    }

    battle_state.second_socket_move = move_res.TakeOk();
  } else {
    return ResultT{
        Error::From(kBattleSocketIdNotFound,
                    core::JsonParser{}.Set("socket_id", socket_id).IntoMap())};
  }

  if (battle_state.first_socket_move.empty() ||
      battle_state.second_socket_move.empty()) {
    return ResultT{Void{}};
  }

  // battle logic
  const auto &first_move = battle_state.first_socket_move;
  const auto &second_move = battle_state.second_socket_move;
  if (first_move == second_move) {
    // send draw
    event_loop.SendServerEvent(battle_state.first_socket_id,
                               core::JsonParser{}
                                   .Set("kind", "battle_result")
                                   .Set("result", "draw")
                                   .Take());
    event_loop.SendServerEvent(battle_state.second_socket_id,
                               core::JsonParser{}
                                   .Set("kind", "battle_result")
                                   .Set("result", "draw")
                                   .Take());
    return ResultT{Void{}};
  }

  const auto &first_win_moves = kRules.at(first_move);
  if (first_win_moves.end() !=
      std::find(first_win_moves.begin(), first_win_moves.end(), second_move)) {
    // send first win
    event_loop.SendServerEvent(battle_state.first_socket_id,
                               core::JsonParser{}
                                   .Set("kind", "battle_result")
                                   .Set("result", "win")
                                   .Take());
    event_loop.SendServerEvent(battle_state.second_socket_id,
                               core::JsonParser{}
                                   .Set("kind", "battle_result")
                                   .Set("result", "lose")
                                   .Take());
    return ResultT{Void{}};
  }

  const auto &second_win_moves = kRules.at(second_move);
  if (second_win_moves.end() !=
      std::find(second_win_moves.begin(), second_win_moves.end(), first_move)) {
    // send second win
    event_loop.SendServerEvent(battle_state.first_socket_id,
                               core::JsonParser{}
                                   .Set("kind", "battle_result")
                                   .Set("result", "lose")
                                   .Take());
    event_loop.SendServerEvent(battle_state.second_socket_id,
                               core::JsonParser{}
                                   .Set("kind", "battle_result")
                                   .Set("result", "win")
                                   .Take());
    return ResultT{Void{}};
  }

  // return error
  return ResultT{Error::From(kBattleMoveLogicError,
                             core::JsonParser{}
                                 .Set("first_move", first_move)
                                 .Set("second_move", second_move)
                                 .IntoMap())};
}
