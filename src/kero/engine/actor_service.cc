#include "actor_service.h"

#include "kero/engine/runner.h"

using namespace kero;

kero::ActorService::ActorService(Pin<RunnerContext> runner_context,
                                 std::string &&name,
                                 MailBox &&mail_box) noexcept
    : Service{runner_context, kServiceKindActor, {}},
      mail_box_{std::move(mail_box)},
      name_{std::move(name)} {}

auto
kero::ActorService::OnUpdate() noexcept -> void {
  auto mail = mail_box_.rx.TryReceive();
  if (mail.IsNone()) {
    return;
  }

  auto [from, to, event, body] = mail.TakeUnwrap();
  GetRunnerContext().InvokeEvent(event,
                                 body.Set("__from", std::string{from})
                                     .Set("__to", std::string{to})
                                     .Take());
}

auto
kero::ActorService::GetName() const noexcept -> const std::string & {
  return name_;
}

auto
kero::ActorService::SendMail(std::string &&to,
                             std::string &&event,
                             Dict &&body) noexcept -> void {
  mail_box_.tx.Send(Mail{std::string{name_},
                         std::move(to),
                         std::move(event),
                         std::move(body)});
}
