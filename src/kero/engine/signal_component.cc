#include "signal_component.h"

#include <signal.h>

#include "kero/engine/actor_system.h"
#include "kero/engine/agent.h"
#include "kero/engine/constants.h"
#include "kero/engine/utils_linux.h"

using namespace kero;

kero::SignalComponent::SignalComponent() noexcept
    : Component{ComponentKind::kSignal} {}

auto
kero::SignalComponent::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  interrupted_ = false;

  if (!agent.HasComponentIs<Actor>(ComponentKind::kActor)) {
    return ResultT::Err(Error::From(kActorComponentNotFound));
  }

  if (signal(SIGINT, OnSignal) == SIG_ERR) {
    return ResultT::Err(Error::From(Errno::FromErrno().IntoDict()));
  }

  return ResultT::Ok(Void{});
}

auto
kero::SignalComponent::OnDestroy(Agent& agent) noexcept -> void {
  if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
    // TODO: log error using errno
    const auto err = Errno::FromErrno();
  }
}

auto
kero::SignalComponent::OnUpdate(Agent& agent) noexcept -> void {
  if (interrupted_) {
    auto actor = agent.GetComponentAs<Actor>(ComponentKind::kActor);
    auto from = actor ? actor.Unwrap().GetName() : "unknown";

    agent.Dispatch(EventMailToSend::kEvent,
                   Dict{}
                       .Set(EventMailToSend::kFrom, std::move(from))
                       .Set(EventMailToSend::kTo, std::string{"all"})
                       .Take());

    interrupted_ = false;
  }
}

auto
kero::SignalComponent::IsInterrupted() const noexcept -> bool {
  return interrupted_;
}

std::atomic<bool> SignalComponent::interrupted_{false};

auto
kero::SignalComponent::OnSignal(int signal) noexcept -> void {
  if (signal == SIGINT) {
    interrupted_ = true;
  }
}
