#include "actor_service.h"

#include "kero/engine/agent.h"
#include "kero/engine/constants.h"

using namespace kero;

kero::ActorService::ActorService(std::string &&name,
                                 MailBox &&mail_box) noexcept
    : Service{ServiceKind::kActor, {}},
      mail_box_{std::move(mail_box)},
      name_{std::move(name)} {}

auto
kero::ActorService::OnCreate(Agent &agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  return ResultT::Ok(Void{});
}

auto
kero::ActorService::OnUpdate(Agent &agent) noexcept -> void {
  auto mail = mail_box_.rx.TryReceive();
  if (mail.IsNone()) {
    return;
  }

  auto [from, to, event, body] = mail.TakeUnwrap();
  agent.Invoke(event,
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
