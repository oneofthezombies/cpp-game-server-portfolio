#include "lobby.h"

#include "server/engine/socket.h"

using namespace contents;

auto
contents::Lobby::OnInit(const engine::EventLoop &event_loop,
                        const engine::Config &config) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Super::OnInit(event_loop, config); res.IsErr()) {
    return res;
  }

  mail_handlers_.emplace("connect", OnConnect);

  return ResultT{Void{}};
}

auto
contents::Lobby::OnMail(const engine::EventLoop &event_loop,
                        const engine::Mail &mail) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Super::OnMail(event_loop, mail); res.IsErr()) {
    return res;
  }

  return ResultT{Void{}};
}

auto
contents::Lobby::OnSocketIn(const engine::EventLoop &event_loop,
                            const engine::SocketId socket_id) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Super::OnSocketIn(event_loop, socket_id); res.IsErr()) {
    return res;
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
contents::Lobby::OnConnect(Lobby &self,
                           const engine::EventLoop &event_loop,
                           const engine::Mail &mail) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  // TODO
  // auto socket_id_res = mail.body.GetAsNumber<engine::SocketId>("socket_id");
  // if (socket_id_res.IsErr()) {
  //   return ResultT{Error::From(kLobbyMailSocketIdParsingFailed,
  //                              core::TinyJson{}
  //                                  .Set("mail", mail)
  //                                  .Set("error", socket_id_res.Err())
  //                                  .IntoMap())};
  // }

  // const auto socket_id = socket_id_res.Ok();

  // TODO
  // event_loop.Write(socket_id, "");

  if (self.IsMatchable()) {
    const auto first_socket_id = *self.sockets_.begin();
    const auto second_socket_id = *std::next(self.sockets_.begin());
    if (auto res = self.UnregisterSocket(event_loop, first_socket_id);
        res.IsErr()) {
      return res;
    }

    if (auto res = self.UnregisterSocket(event_loop, second_socket_id);
        res.IsErr()) {
      return res;
    }

    const auto battle_id = self.NextBattleId();
    event_loop.SendMail(
        "battle",
        std::move(core::TinyJson{}
                      .Set("kind", "start")
                      .Set("battle_id", battle_id)
                      .Set("first_socket_id", first_socket_id)
                      .Set("second_socket_id", second_socket_id)));
  }

  return ResultT{Void{}};
}
