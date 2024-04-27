#include "socket_event_loop_handler.h"

#include <sys/epoll.h>

#include "core/utils.h"

#include "socket.h"

using namespace engine;

auto engine::SocketEventLoopHandler::OnInit(const EventLoop &event_loop,
                                            const Config &config) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  // TODO

  return ResultT{Void{}};
}

auto engine::SocketEventLoopHandler::OnMail(const EventLoop &event_loop,
                                            Mail &&mail) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto socket_id_str = mail.body.Get("socket_id")) {
    auto socket_id_res = core::ParseNumberString<SocketId>(*socket_id_str);
    if (socket_id_res.IsErr()) {
      return ResultT{Error{
          Symbol::kSocketEventLoopHandlerParseSocketIdFailed,
          core::TinyJson{}
              .Set("errc", std::make_error_code(socket_id_res.Err()).message())
              .ToString()}};
    }

    const auto socket_id = socket_id_res.Ok();
    if (sessions_.find(socket_id) != sessions_.end()) {
      return ResultT{
          Error{Symbol::kSocketEventLoopHandlerSocketIdAlreadyExists,
                core::TinyJson{}.Set("socket_id", socket_id).ToString()}};
    }

    if (auto res = event_loop.Add(socket_id, EPOLLIN | EPOLLET); res.IsErr()) {
      return res;
    }

    sessions_.emplace(socket_id);
  }

  return ResultT{Void{}};
}

auto engine::SocketEventLoopHandler::OnSocketIn(
    const EventLoop &event_loop, const SocketId socket_id) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  // TODO

  return ResultT{Void{}};
}

auto engine::SocketEventLoopHandler::OnSocketHangUp(
    const EventLoop &event_loop, const SocketId socket_id) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  const auto found = sessions_.find(socket_id);
  if (found == sessions_.end()) {
    return ResultT{
        Error{Symbol::kSocketEventLoopHandlerSocketIdNotFound,
              core::TinyJson{}.Set("socket_id", socket_id).ToString()}};
  }

  sessions_.erase(found);
  return ResultT{Void{}};
}

auto engine::SocketEventLoopHandler::OnSocketError(
    const EventLoop &event_loop, const SocketId socket_id, const int code,
    const std::string_view description) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  core::TinyJson{}
      .Set("reason", "socket error")
      .Set("name", event_loop.GetName())
      .Set("socket_id", socket_id)
      .Set("code", code)
      .Set("description", description)
      .LogLn();
  return ResultT{Void{}};
}
