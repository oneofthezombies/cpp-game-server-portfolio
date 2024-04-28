#include "battle.h"

#include "core/tiny_json.h"

using namespace contents;

auto
contents::Battle::OnInit(engine::EventLoop &event_loop,
                         const engine::Config &config) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Super::OnInit(event_loop, config); res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  mail_handlers_.emplace("start", OnStart);

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
          core::TinyJson{}
              .Set("kind", "battle_start")
              .Set("battle_id", battle_id)
              .Set("opponent_socket_id", second_socket_id)
              .Take());
      res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  if (auto res = event_loop.SendServerEvent(
          second_socket_id,
          core::TinyJson{}
              .Set("kind", "battle_start")
              .Set("battle_id", battle_id)
              .Set("opponent_socket_id", first_socket_id)
              .Take());
      res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  return ResultT{Void{}};
}
