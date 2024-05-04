#include "actor_service.h"

#include "kero/engine/constants.h"
#include "kero/engine/engine.h"
#include "kero/engine/runner_context.h"
#include "kero/log/log_builder.h"

using namespace kero;

auto
kero::ActorService::GetKindId() noexcept -> ServiceKindId {
  return kServiceKindIdActor;
}

auto
kero::ActorService::GetKindName() noexcept -> ServiceKindName {
  return "actor";
}

kero::ActorService::ActorService(const Pin<RunnerContext> runner_context,
                                 std::string &&name,
                                 MailBox &&mail_box) noexcept
    : Service{runner_context, {}},
      mail_box_{std::move(mail_box)},
      name_{std::move(name)} {}

auto
kero::ActorService::OnUpdate() noexcept -> void {
  auto mail = mail_box_.rx.TryReceive();
  if (mail.IsNone()) {
    return;
  }

  auto [from, to, event, body] = mail.TakeUnwrap();
  if (auto res = InvokeEvent(event,
                             body.Set("__from", std::string{from})
                                 .Set("__to", std::string{to})
                                 .Take())) {
    log::Error("Failed to invoke event")
        .Data("event", event)
        .Data("from", from)
        .Data("to", to)
        .Log();
    return;
  }
}

auto
kero::ActorService::GetName() const noexcept -> const std::string & {
  return name_;
}

auto
kero::ActorService::SendMail(std::string &&to,
                             std::string &&event,
                             FlatJson &&body) noexcept -> void {
  mail_box_.tx.Send(Mail{std::string{name_},
                         std::move(to),
                         std::move(event),
                         std::move(body)});
}

auto
kero::ActorService::BroadcastMail(std::string &&event,
                                  FlatJson &&body) noexcept -> void {
  mail_box_.tx.Send(
      Mail{std::string{name_}, "broadcast", std::move(event), std::move(body)});
}

kero::ActorServiceFactory::ActorServiceFactory(
    const Own<Engine> &engine) noexcept
    : engine_{engine} {}

auto
kero::ActorServiceFactory::Create(
    const Pin<RunnerContext> runner_context) noexcept -> Result<Own<Service>> {
  using ResultT = Result<Own<Service>>;

  auto name = runner_context->GetName();
  auto mail_box_res =
      engine_->engine_context_->actor_system.CreateMailBox(name);
  if (mail_box_res.IsErr()) {
    return ResultT::Err(mail_box_res.TakeErr());
  }

  auto mail_box = mail_box_res.TakeOk();
  return ResultT::Ok(Own<ActorService>{
      new ActorService{runner_context, std::move(name), std::move(mail_box)}});
}
