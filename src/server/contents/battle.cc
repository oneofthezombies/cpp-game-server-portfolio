#include "battle.h"

#include "core/tiny_json.h"
#include "core/utils.h"

using namespace contents;

auto
contents::Battle::OnInit(engine::EventLoop &event_loop,
                         const engine::Config &config) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

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

  return ResultT{Void{}};
}

auto
contents::Battle::OnStart(engine::EventLoop &event_loop,
                          const engine::Mail &mail) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  return ResultT{Void{}};
}
