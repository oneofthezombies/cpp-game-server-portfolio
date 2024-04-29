#include "signal_service.h"

#include <signal.h>

#include "kero/core/utils_linux.h"
#include "kero/engine/actor_system.h"
#include "kero/engine/agent.h"
#include "kero/engine/constants.h"
#include "kero/log/log_builder.h"

using namespace kero;

kero::SignalService::SignalService() noexcept : Service{ServiceKind::kSignal} {}

auto
kero::SignalService::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  interrupted_ = false;

  if (!agent.HasServiceIs<ActorService>(ServiceKind::kActor)) {
    return ResultT::Err(Error::From(
        Dict{}.Set("message", std::string{"ActorService not found"}).Take()));
  }

  if (signal(SIGINT, OnSignal) == SIG_ERR) {
    return ResultT::Err(Error::From(Errno::FromErrno().IntoDict()));
  }

  return ResultT::Ok(Void{});
}

auto
kero::SignalService::OnDestroy(Agent& agent) noexcept -> void {
  if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
    log::Error("Failed to reset signal handler")
        .Data("errno", Errno::FromErrno())
        .Log();
  }
}

auto
kero::SignalService::OnUpdate(Agent& agent) noexcept -> void {
  if (!interrupted_) {
    return;
  }

  auto actor = agent.GetServiceAs<ActorService>(ServiceKind::kActor);
  if (!actor) {
    log::Error("Failed to get ActorService").Log();
    return;
  }

  actor.Unwrap().SendMail("all", EventShutdown::kEvent, Dict{});

  interrupted_ = false;
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
