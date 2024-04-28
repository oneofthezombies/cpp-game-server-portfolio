#include "event_loop.h"

#include "core/tiny_json.h"

using namespace engine;

engine::EventLoop::EventLoop(MailBox &&mail_box,
                             std::string &&name,
                             EventLoopHandlerPtr &&handler) noexcept
    : mail_box_{std::move(mail_box)},
      name_{std::move(name)},
      handler_{std::move(handler)} {}

auto
engine::EventLoop::GetMailBox() const noexcept -> const MailBox & {
  return mail_box_;
}

auto
engine::EventLoop::GetName() const noexcept -> std::string_view {
  return name_;
}

auto
engine::EventLoop::SendMail(const std::string_view to,
                            MailBody &&mail_body) const noexcept -> void {
  mail_box_.tx.Send(
      Mail{std::string{GetName()}, std::string{to}, std::move(mail_body)});
}

auto
engine::EventLoop::SendServerEvent(const SocketId socket_id,
                                   core::Message &&message) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = message.Get("__message_kind"); res.IsOk()) {
    return ResultT{Error::From(kEventLoopMessageKindDuplicated,
                               core::TinyJson{}
                                   .Set("socket_id", socket_id)
                                   .Set("message", message)
                                   .IntoMap())};
  }

  if (auto res = message.Get("__message_id"); res.IsOk()) {
    return ResultT{Error::From(kEventLoopMessageIdDuplicated,
                               core::TinyJson{}
                                   .Set("socket_id", socket_id)
                                   .Set("message", message)
                                   .IntoMap())};
  }

  const auto data =
      message
          .Set("__message_kind",
               static_cast<int32_t>(core::MessageKind::kServerEvent))
          .Set("__message_id", NextMessageId())
          .ToString();

  if (auto res = Write(socket_id, data); res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  return ResultT{Void{}};
}

auto
engine::EventLoop::NextMessageId() noexcept -> core::MessageId {
  return next_message_id_++;
}
