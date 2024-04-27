#include "lobby.h"

using namespace contents;

auto contents::Lobby::OnInit(const engine::EventLoop &event_loop,
                             const engine::Config &config) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  return ResultT{Void{}};
}

auto contents::Lobby::OnMail(const engine::EventLoop &event_loop,
                             const engine::Mail &mail) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Super::OnMail(event_loop, mail); res.IsErr()) {
    return res;
  }

  if (IsMatchable()) {
    const auto first_socket_id = *sockets_.begin();
    const auto second_socket_id = *std::next(sockets_.begin());
    if (auto res = UnregisterSocket(event_loop, first_socket_id); res.IsErr()) {
      return res;
    }

    if (auto res = UnregisterSocket(event_loop, second_socket_id);
        res.IsErr()) {
      return res;
    }

    const auto battle_id = NextBattleId();
    event_loop.SendMail(
        "battle", std::move(core::TinyJson{}
                                .Set("kind", "start")
                                .Set("battle_id", battle_id)
                                .Set("first_socket_id", first_socket_id)
                                .Set("second_socket_id", second_socket_id)));
  }

  return ResultT{Void{}};
}

auto contents::Lobby::OnSocketIn(const engine::EventLoop &event_loop,
                                 const engine::SocketId socket_id) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  return ResultT{Void{}};
}

auto contents::Lobby::IsMatchable() const noexcept -> bool {
  return sockets_.size() >= 2;
}

auto contents::Lobby::NextBattleId() noexcept -> uint64_t {
  return next_battle_id_++;
}
