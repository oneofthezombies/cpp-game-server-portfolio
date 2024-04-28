#include "lobby.h"

#include "core/tiny_json.h"

using namespace contents;

auto
contents::Lobby::OnInit(engine::EventLoop &event_loop,
                        const engine::Config &config) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Super::OnInit(event_loop, config); res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  mail_handlers_.emplace("connect", OnConnect);

  return ResultT{Void{}};
}

auto
contents::Lobby::OnMail(engine::EventLoop &event_loop,
                        const engine::Mail &mail) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Super::OnMail(event_loop, mail); res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  return ResultT{Void{}};
}

auto
contents::Lobby::OnSocketIn(engine::EventLoop &event_loop,
                            const engine::SocketId socket_id) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Super::OnSocketIn(event_loop, socket_id); res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  return ResultT{Void{}};
}

auto
contents::Lobby::IsMatchable() const noexcept -> bool {
  return sockets_.size() >= 2;
}

auto
contents::Lobby::NextBattleId() noexcept -> BattleId {
  return next_battle_id_++;
}

auto
contents::Lobby::OnConnect(Self &self,
                           engine::EventLoop &event_loop,
                           const engine::Mail &mail) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  auto socket_id_res = mail.body.GetAsNumber<engine::SocketId>("socket_id");
  if (socket_id_res.IsErr()) {
    return ResultT{Error::From(kLobbyMailGetSocketIdFailed,
                               core::TinyJson{}.Set("mail", mail).IntoMap(),
                               socket_id_res.TakeErr())};
  }

  const auto socket_id = socket_id_res.Ok();
  auto message = core::TinyJson{}
                     .Set("kind", "connect")
                     .Set("socket_id", socket_id)
                     .Take();
  if (auto res = event_loop.SendServerEvent(socket_id, std::move(message));
      res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  if (self.IsMatchable()) {
    const auto first_socket_id = *self.sockets_.begin();
    const auto second_socket_id = *std::next(self.sockets_.begin());
    if (auto res = self.UnregisterSocket(event_loop, first_socket_id);
        res.IsErr()) {
      return ResultT{Error::From(res.TakeErr())};
    }

    if (auto res = self.UnregisterSocket(event_loop, second_socket_id);
        res.IsErr()) {
      return ResultT{Error::From(res.TakeErr())};
    }

    const auto battle_id = self.NextBattleId();
    event_loop.SendMail("battle",
                        core::TinyJson{}
                            .Set("kind", "start")
                            .Set("battle_id", battle_id)
                            .Set("first_socket_id", first_socket_id)
                            .Set("second_socket_id", second_socket_id)
                            .Take());
  }

  return ResultT{Void{}};
}
