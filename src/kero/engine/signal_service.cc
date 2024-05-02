#include "signal_service.h"

#include <signal.h>

#include "kero/core/utils_linux.h"
#include "kero/engine/actor_service.h"
#include "kero/engine/constants.h"
#include "kero/engine/runner_context.h"
#include "kero/log/log_builder.h"

using namespace kero;

kero::SignalService::SignalService(
    const Pinned<RunnerContext> runner_context) noexcept
    : Service{runner_context, kServiceKindSignal, {kServiceKindActor}} {}

auto
kero::SignalService::OnCreate() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  interrupted_ = false;

  if (signal(SIGINT, OnSignal) == SIG_ERR) {
    return ResultT::Err(Errno::FromErrno().IntoJson());
  }

  return OkVoid();
}

auto
kero::SignalService::OnDestroy() noexcept -> void {
  if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
    log::Error("Failed to reset signal handler")
        .Data("errno", Errno::FromErrno())
        .Log();
  }
}

auto
kero::SignalService::OnUpdate() noexcept -> void {
  if (!interrupted_) {
    return;
  }

  auto actor = GetRunnerContext()
                   .GetService(kServiceKindActor.id)
                   .Unwrap()
                   .As<ActorService>(kServiceKindActor.id);
  if (!actor) {
    log::Error("Failed to get ActorService").Log();
    return;
  }

  actor.Unwrap().SendMail("all", EventShutdown::kEvent, Json{});
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
