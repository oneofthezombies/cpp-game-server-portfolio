#ifndef SERVER_ENGINE_SOCKET_EVENT_LOOP_HANDLER_H
#define SERVER_ENGINE_SOCKET_EVENT_LOOP_HANDLER_H

#include <unordered_set>

#include "core/core.h"
#include "core/protocol.h"
#include "event_loop.h"
#include "server/engine/common.h"

namespace engine {

template <typename T>
class SocketEventLoopHandler : public EventLoopHandler {
 public:
  using Super = EventLoopHandler;
  using Derived = T;
  using Kind = std::string;
  using MailHandler =
      std::function<Result<Void>(Derived &, EventLoop &, const Mail &)>;
  using MessageHandler = std::function<Result<Void>(
      Derived &, EventLoop &, const SocketId, core::Message &&)>;

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

    auto message_res = event_loop.Read(socket_id);
    if (message_res.IsErr()) {
      if (message_res.Err().code == kEventLoopLinuxReadClosed) {
        if (auto res = OnSocketClose(event_loop, socket_id, "socket closed");
            res.IsErr()) {
          return ResultT{Error::From(res.TakeErr())};
        }

        return ResultT{Void{}};
      }

      return ResultT{Error::From(message_res.TakeErr())};
    }

    const auto message = message_res.TakeOk();
    auto parse_res = core::Message::Parse(message);
    if (!parse_res) {
      return ResultT{Error::From(kSocketEventLoopHandlerMessageParseFailed,
                                 core::TinyJson{}
                                     .Set("message", message)
                                     .Set("name", event_loop.GetName())
                                     .IntoMap())};
    }

    auto kind_res = parse_res->Get("kind");
    if (kind_res.IsErr()) {
      return ResultT{Error::From(kSocketEventLoopHandlerMessageKindNotFound,
                                 core::TinyJson{}
                                     .Set("message", message)
                                     .Set("name", event_loop.GetName())
                                     .IntoMap(),
                                 kind_res.TakeErr())};
    }

    const auto kind = kind_res.Ok();
    const auto handler = message_handlers_.find(std::string{kind});
    if (handler == message_handlers_.end()) {
      return ResultT{Error::From(kSocketEventLoopHandlerMessageHandlerNotFound,
                                 core::TinyJson{}
                                     .Set("message", message)
                                     .Set("name", event_loop.GetName())
                                     .IntoMap())};
    }

    if (auto res = (handler->second)(static_cast<Derived &>(*this),
                                     event_loop,
                                     socket_id,
                                     std::move(*parse_res));
        res.IsErr()) {
      return ResultT{Error::From(kSocketEventLoopHandlerMessageHandlerFailed,
                                 core::TinyJson{}
                                     .Set("message", message)
                                     .Set("name", event_loop.GetName())
                                     .IntoMap(),
                                 res.TakeErr())};
    }

    return ResultT{Void{}};
  }

  [[nodiscard]] virtual auto
  OnSocketHangUp(EventLoop &event_loop,
                 const SocketId socket_id) noexcept -> Result<Void> override {
    using ResultT = Result<Void>;

    if (auto res = OnSocketClose(event_loop, socket_id, "socket hang up");
        res.IsErr()) {
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

  [[nodiscard]] auto
  OnSocketClose(EventLoop &event_loop,
                const SocketId socket_id,
                const std::string_view message) noexcept -> Result<Void> {
    using ResultT = Result<Void>;

    if (auto res = UnregisterSocket(event_loop, socket_id); res.IsErr()) {
      core::TinyJson{}
          .Set("message", message)
          .Set("name", event_loop.GetName())
          .Set("socket_id", socket_id)
          .Set("error", res.TakeErr())
          .LogLn();
    }

    // call callback

    return ResultT{Void{}};
  }

 protected:
  std::unordered_set<SocketId> sockets_;
  std::unordered_map<Kind, MailHandler> mail_handlers_;
  std::unordered_map<Kind, MessageHandler> message_handlers_;
};

}  // namespace engine

#endif  // SERVER_ENGINE_SOCKET_EVENT_LOOP_HANDLER_H
