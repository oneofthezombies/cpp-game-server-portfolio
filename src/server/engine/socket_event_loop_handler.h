#ifndef SERVER_ENGINE_SOCKET_EVENT_LOOP_HANDLER_H
#define SERVER_ENGINE_SOCKET_EVENT_LOOP_HANDLER_H

#include <unordered_set>

#include "core/core.h"
#include "event_loop.h"

namespace engine {

template <typename T>
class SocketEventLoopHandler : public EventLoopHandler {
 public:
  using Super = EventLoopHandler;
  using Derived = T;
  using MailHandler =
      std::function<Result<Void>(Derived &, EventLoop &, const Mail &)>;

  explicit SocketEventLoopHandler() noexcept = default;
  virtual ~SocketEventLoopHandler() noexcept = default;
  CLASS_KIND_MOVABLE(SocketEventLoopHandler);

  [[nodiscard]] virtual auto
  OnInit(EventLoop &event_loop,
         const Config &config) noexcept -> Result<Void> override {
    using ResultT = Result<Void>;

    // noop

    return ResultT{Void{}};
  }

  [[nodiscard]] virtual auto
  OnMail(EventLoop &event_loop,
         const Mail &mail) noexcept -> Result<Void> override {
    using ResultT = Result<Void>;

    if (mail.from == "main") {
      auto socket_id_res = mail.body.GetAsNumber<SocketId>("socket_id");
      if (socket_id_res.IsErr()) {
        return ResultT{Error::From(kSocketEventLoopHandlerMailSocketIdNotFound,
                                   core::TinyJson{}.Set("mail", mail).IntoMap(),
                                   socket_id_res.TakeErr())};
      }

      const auto socket_id = socket_id_res.Ok();
      if (auto res = RegisterSocket(event_loop, socket_id); res.IsErr()) {
        return ResultT{Error::From(res.TakeErr())};
      }
    }

    auto kind_res = mail.body.Get("kind");
    if (kind_res.IsErr()) {
      return ResultT{Error::From(kSocketEventLoopHandlerMailKindNotFound,
                                 core::TinyJson{}.Set("mail", mail).IntoMap(),
                                 kind_res.TakeErr())};
    }

    const auto kind = kind_res.Ok();
    const auto handler = mail_handlers_.find(std::string{kind});
    if (handler == mail_handlers_.end()) {
      return ResultT{Error::From(kSocketEventLoopHandlerMailHandlerNotFound,
                                 core::TinyJson{}
                                     .Set("mail", mail)
                                     .Set("name", event_loop.GetName())
                                     .IntoMap())};
    }

    if (auto res =
            (handler->second)(static_cast<Derived &>(*this), event_loop, mail);
        res.IsErr()) {
      return ResultT{Error::From(kSocketEventLoopHandlerMailHandlerFailed,
                                 core::TinyJson{}
                                     .Set("mail", mail)
                                     .Set("name", event_loop.GetName())
                                     .IntoMap(),
                                 res.TakeErr())};
    }

    return ResultT{Void{}};
  }

  [[nodiscard]] virtual auto
  OnSocketIn(EventLoop &event_loop,
             const SocketId socket_id) noexcept -> Result<Void> override {
    using ResultT = Result<Void>;

    // noop

    return ResultT{Void{}};
  }

  [[nodiscard]] virtual auto
  OnSocketHangUp(EventLoop &event_loop,
                 const SocketId socket_id) noexcept -> Result<Void> override {
    using ResultT = Result<Void>;

    if (auto res = UnregisterSocket(event_loop, socket_id); res.IsErr()) {
      return ResultT{Error::From(res.TakeErr())};
    }

    return ResultT{Void{}};
  }

  [[nodiscard]] virtual auto
  OnSocketError(EventLoop &event_loop,
                const SocketId socket_id,
                const int code,
                const std::string_view description) noexcept
      -> Result<Void> override {
    using ResultT = Result<Void>;

    core::TinyJson{}
        .Set("message", "socket error")
        .Set("name", event_loop.GetName())
        .Set("socket_id", socket_id)
        .Set("code", code)
        .Set("description", description)
        .LogLn();
    return ResultT{Void{}};
  }

  [[nodiscard]] auto
  RegisterSocket(EventLoop &event_loop,
                 const SocketId socket_id) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    if (auto res =
            event_loop.Add(socket_id, {.in = true, .edge_trigger = true});
        res.IsErr()) {
      return ResultT{Error::From(res.TakeErr())};
    }

    if (auto res = AddSocketToSet(socket_id); res.IsErr()) {
      return ResultT{Error::From(res.TakeErr())};
    }

    return ResultT{Void{}};
  }

  [[nodiscard]] auto
  UnregisterSocket(EventLoop &event_loop,
                   const SocketId socket_id) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    if (auto res = RemoveSocketFromSet(socket_id); res.IsErr()) {
      return ResultT{Error::From(res.TakeErr())};
    }

    if (auto res = event_loop.Remove(socket_id); res.IsErr()) {
      return ResultT{Error::From(res.TakeErr())};
    }

    return ResultT{Void{}};
  }

 private:
  [[nodiscard]] auto
  AddSocketToSet(const SocketId socket_id) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    if (sockets_.find(socket_id) != sockets_.end()) {
      return ResultT{
          Error::From(kSocketEventLoopHandlerSocketIdAlreadyExists,
                      core::TinyJson{}.Set("socket_id", socket_id).IntoMap())};
    }

    sockets_.emplace(socket_id);
    return ResultT{Void{}};
  }

  [[nodiscard]] auto
  RemoveSocketFromSet(const SocketId socket_id) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    const auto found = sockets_.find(socket_id);
    if (found == sockets_.end()) {
      return ResultT{
          Error::From(kSocketEventLoopHandlerSocketIdNotFound,
                      core::TinyJson{}.Set("socket_id", socket_id).IntoMap())};
    }

    sockets_.erase(found);
    return ResultT{Void{}};
  }

  [[nodiscard]] auto
  RegisterMailHandler(std::string &&kind,
                      MailHandler &&handler) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    if (mail_handlers_.find(kind) != mail_handlers_.end()) {
      return ResultT{
          Error::From(kSocketEventLoopHandlerMailHandlerAlreadyExists,
                      core::TinyJson{}.Set("kind", kind).IntoMap())};
    }

    mail_handlers_.emplace(kind, handler);
    return ResultT{Void{}};
  }

 protected:
  std::unordered_set<SocketId> sockets_;
  std::unordered_map<std::string, MailHandler> mail_handlers_;
};

}  // namespace engine

#endif  // SERVER_ENGINE_SOCKET_EVENT_LOOP_HANDLER_H
