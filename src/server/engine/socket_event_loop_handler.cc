#include "socket_event_loop_handler.h"

#include <sys/epoll.h>

#include "core/utils.h"

#include "socket.h"

using namespace engine;

auto engine::SocketEventLoopHandler::OnMail(const EventLoop &event_loop,
                                            const Mail &mail) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (mail.from == "main") {
    if (auto socket_id_str = mail.body.Get("socket_id")) {
      auto socket_id_res = core::ParseNumberString<SocketId>(*socket_id_str);
      if (socket_id_res.IsErr()) {
        return ResultT{
            Error{Symbol::kSocketEventLoopHandlerParseSocketIdFailed,
                  core::TinyJson{}
                      .Set("errc",
                           std::make_error_code(socket_id_res.Err()).message())
                      .ToString()}};
      }

      const auto socket_id = socket_id_res.Ok();
      if (auto res = RegisterSocket(event_loop, socket_id); res.IsErr()) {
        return res;
      }
    }
  }

  return ResultT{Void{}};
}

auto engine::SocketEventLoopHandler::OnSocketHangUp(
    const EventLoop &event_loop, const SocketId socket_id) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = UnregisterSocket(event_loop, socket_id); res.IsErr()) {
    return res;
  }

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

auto engine::SocketEventLoopHandler::AddSocketToSet(
    const SocketId socket_id) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (sockets_.find(socket_id) != sockets_.end()) {
    return ResultT{
        Error{Symbol::kSocketEventLoopHandlerSocketIdAlreadyExists,
              core::TinyJson{}.Set("socket_id", socket_id).ToString()}};
  }

  sockets_.emplace(socket_id);
  return ResultT{Void{}};
}

auto engine::SocketEventLoopHandler::RemoveSocketFromSet(
    const SocketId socket_id) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  const auto found = sockets_.find(socket_id);
  if (found == sockets_.end()) {
    return ResultT{
        Error{Symbol::kSocketEventLoopHandlerSocketIdNotFound,
              core::TinyJson{}.Set("socket_id", socket_id).ToString()}};
  }

  sockets_.erase(found);
  return ResultT{Void{}};
}

auto engine::SocketEventLoopHandler::RegisterSocket(
    const EventLoop &event_loop, const SocketId socket_id) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = event_loop.Add(socket_id, EPOLLIN | EPOLLET); res.IsErr()) {
    return res;
  }

  if (auto res = AddSocketToSet(socket_id); res.IsErr()) {
    return res;
  }

  return ResultT{Void{}};
}

auto engine::SocketEventLoopHandler::UnregisterSocket(
    const EventLoop &event_loop, const SocketId socket_id) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = RemoveSocketFromSet(socket_id); res.IsErr()) {
    return res;
  }

  if (auto res = event_loop.Delete(socket_id); res.IsErr()) {
    return res;
  }

  return ResultT{Void{}};
}
