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

  if (sockets_.size() >= 2) {
    const auto first = *sockets_.begin();
    const auto second = *std::next(sockets_.begin());
    if (auto res = UnregisterSocket(event_loop, first); res.IsErr()) {
      return res;
    }

    if (auto res = UnregisterSocket(event_loop, second); res.IsErr()) {
      return res;
    }

    const auto battle_id = NextBattleId();
    event_loop.SendMail("battle_start",
                        std::move(core::TinyJson{}
                                      .Set("battle_id", battle_id)
                                      .Set("first", first)
                                      .Set("second", second)));
  }

  return ResultT{Void{}};
}

auto contents::Lobby::OnSocketIn(const engine::EventLoop &event_loop,
                                 const engine::SocketId socket_id) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  return ResultT{Void{}};
}

auto contents::Lobby::NextBattleId() noexcept -> uint64_t {
  return next_battle_id_++;
}