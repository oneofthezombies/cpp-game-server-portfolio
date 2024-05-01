#include "signal_service.h"

#include <signal.h>

#include "kero/core/utils_linux.h"
#include "kero/engine/agent.h"
#include "kero/log/log_builder.h"
#include "kero/service/actor_service.h"
#include "kero/service/constants.h"

using namespace kero;

kero::SignalService::SignalService(
    const Pin<RunnerContext> runner_context) noexcept
    : Service{runner_context, kServiceKindSignal, {}} {}

auto
kero::SignalService::OnCreate(Agent& agent) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  interrupted_ = false;

  if (signal(SIGINT, OnSignal) == SIG_ERR) {
    return ResultT::Err(Error::From(Errno::FromErrno().IntoDict()));
  }

  return OkVoid;
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
  log::Debug("Signal received").Data("signal", signal).Log();
}
