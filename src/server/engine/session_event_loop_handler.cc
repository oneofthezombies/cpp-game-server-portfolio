#include "session_event_loop_handler.h"

using namespace engine;

auto engine::SessionEventLoopHandler::OnInit(
    const Config &config, const EventLoop &event_loop) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  // TODO

  return ResultT{Void{}};
}

auto engine::SessionEventLoopHandler::OnMail(const EventLoopContext &context,
                                             Mail &&mail) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  // TODO

  return ResultT{Void{}};
}

auto engine::SessionEventLoopHandler::OnSessionEvent(
    const EventLoopContext &context, const SessionId session_id,
    const uint32_t events) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  // TODO

  return ResultT{Void{}};
}
