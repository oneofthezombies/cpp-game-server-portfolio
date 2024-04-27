#include "battle.h"

#include "core/tiny_json.h"
#include "core/utils.h"

using namespace contents;

auto contents::Battle::OnInit(const engine::EventLoop &event_loop,
                              const engine::Config &config) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  return ResultT{Void{}};
}

auto contents::Battle::OnMail(const engine::EventLoop &event_loop,
                              const engine::Mail &mail) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Super::OnMail(event_loop, mail); res.IsErr()) {
    return res;
  }

  auto kind = mail.body.Get("kind");
  if (!kind) {
    return ResultT{Error{kBattleMailKindNotFound,
                         core::TinyJson{}.Set("mail", mail).IntoMap()}};
  }

  if (kind == "start") {
    if (auto res = OnStart(event_loop, mail); res.IsErr()) {
      return res;
    }
  } else {
    return ResultT{Error{kBattleMailUnexpectedKind,
                         core::TinyJson{}.Set("mail", mail).IntoMap()}};
  }

  return ResultT{Void{}};
}

auto contents::Battle::OnSocketIn(const engine::EventLoop &event_loop,
                                  const engine::SocketId socket_id) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  return ResultT{Void{}};
}

auto contents::Battle::OnStart(const engine::EventLoop &event_loop,
                               const engine::Mail &mail) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  const auto battle_id_str = mail.body.Get("battle_id");
  if (!battle_id_str) {
    return ResultT{Error{kBattleMailBattleIdNotFound,
                         core::TinyJson{}.Set("mail", mail).IntoMap()}};
  }

  const auto battle_id_res = core::ParseNumberString<uint64_t>(*battle_id_str);
  if (battle_id_res.IsErr()) {
    return ResultT{Error{kBattleMailBattleIdParsingFailed,
                         core::TinyJson{}.Set("mail", mail).IntoMap()}};
  }

  const auto battle_id = battle_id_res.Ok();

  const auto first_socket_id_str = mail.body.Get("first_socket_id");
  if (!first_socket_id_str) {
    return ResultT{Error{kBattleMailFirstSocketIdNotFound,
                         core::TinyJson{}.Set("mail", mail).IntoMap()}};
  }

  const auto first_socket_id_res =
      core::ParseNumberString<engine::SocketId>(*first_socket_id_str);
  if (first_socket_id_res.IsErr()) {
    return ResultT{Error{kBattleMailFirstSocketIdParsingFailed,
                         core::TinyJson{}.Set("mail", mail).IntoMap()}};
  }

  const auto first_socket_id = first_socket_id_res.Ok();

  const auto second_socket_id_str = mail.body.Get("second_socket_id");
  if (!second_socket_id_str) {
    return ResultT{Error{kBattleMailSecondSocketIdNotFound,
                         core::TinyJson{}.Set("mail", mail).IntoMap()}};
  }

  const auto second_socket_id_res =
      core::ParseNumberString<engine::SocketId>(*second_socket_id_str);
  if (second_socket_id_res.IsErr()) {
    return ResultT{Error{kBattleMailSecondSocketIdParsingFailed,
                         core::TinyJson{}.Set("mail", mail).IntoMap()}};
  }

  const auto second_socket_id = second_socket_id_res.Ok();

  {
    BattleState battle_state{};
    battle_state.battle_id = battle_id;
    battle_states_.emplace(first_socket_id, std::move(battle_state));
  }

  {
    BattleState battle_state{};
    battle_state.battle_id = battle_id;
    battle_states_.emplace(second_socket_id, std::move(battle_state));
  }

  event_loop.Write();

  return ResultT{Void{}};
}
