#include "signal_service.h"

#include <signal.h>

#include "kero/core/utils_linux.h"
#include "kero/engine/actor_system.h"
#include "kero/engine/agent.h"
#include "kero/engine/constants.h"

using namespace kero;

kero::SignalService::SignalService() noexcept : Service{ServiceKind::kSignal} {}

auto
kero::SignalService::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  interrupted_ = false;

  if (!agent.HasServiceIs<Actor>(ServiceKind::kActor)) {
    return ResultT::Err(Error::From(kActorServiceNotFound));
  }

  if (signal(SIGINT, OnSignal) == SIG_ERR) {
    return ResultT::Err(Error::From(Errno::FromErrno().IntoDict()));
  }

  return ResultT::Ok(Void{});
}

auto
kero::SignalService::OnDestroy(Agent& agent) noexcept -> void {
  if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
    // TODO: log error using errno
    const auto err = Errno::FromErrno();
  }
}

auto
kero::SignalService::OnUpdate(Agent& agent) noexcept -> void {
  if (interrupted_) {
    auto actor = agent.GetServiceAs<Actor>(ServiceKind::kActor);
    auto from = actor ? actor.Unwrap().GetName() : "unknown";

    agent.Invoke(EventMailToSend::kEvent,
                 Dict{}
                     .Set(EventMailToSend::kFrom, std::move(from))
                     .Set(EventMailToSend::kTo, std::string{"all"})
                     .Take());

    interrupted_ = false;
  }
}

auto
kero::SignalService::IsInterrupted() const noexcept -> bool {
  return interrupted_;
}

std::atomic<bool> SignalService::interrupted_{false};

auto
kero::SignalService::OnSignal(int signal) noexcept -> void {
  if (signal == SIGINT) {
    interrupted_ = true;
  }
}
