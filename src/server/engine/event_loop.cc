#include "event_loop.h"

using namespace engine;

engine::EventLoop::EventLoop(MailBox &&mail_box, std::string &&name,
                             EventLoopHandlerPtr &&handler) noexcept
    : mail_box_{std::move(mail_box)}, name_{std::move(name)},
      handler_{std::move(handler)} {}

auto engine::EventLoop::GetMailBox() const noexcept -> const MailBox & {
  return mail_box_;
}

auto engine::EventLoop::GetName() const noexcept -> std::string_view {
  return name_;
}

auto engine::EventLoop::SendMail(const std::string_view to,
                                 MailBody &&mail_body) const noexcept -> void {
  mail_box_.tx.Send(
      Mail{std::string{GetName()}, std::string{to}, std::move(mail_body)});
}
