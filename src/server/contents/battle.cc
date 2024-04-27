#include "battle.h"
#include "core/tiny_json.h"

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

  const auto battle_id = mail.body.Get("battle_id");
  if (!battle_id) {
    return ResultT{Error{kBattleMailBattleIdNotFound,
                         core::TinyJson{}.Set("mail", mail).IntoMap()}};
  }

  const auto first_socket_id = mail.body.Get("first_socket_id");
  if (!first_socket_id) {
    return ResultT{Error{kBattleMailFirstSocketIdNotFound,
                         core::TinyJson{}.Set("mail", mail).IntoMap()}};
  }

  const auto second_socket_id = mail.body.Get("second_socket_id");
  if (!second_socket_id) {
    return ResultT{Error{kBattleMailSecondSocketIdNotFound,
                         core::TinyJson{}.Set("mail", mail).IntoMap()}};
  }

  // battle_states_[first_socket_id] = BattleState{battle_id};
  // battle_states_[second_socket_id] = BattleState{battle_id};

  return ResultT{Void{}};
}
