#include "runner.h"

#include "kero/engine/constants.h"
#include "kero/engine/runner_builder.h"
#include "kero/engine/runner_context.h"
#include "kero/engine/service.h"
#include "kero/engine/signal_service.h"
#include "kero/log/log_builder.h"

using namespace kero;

kero::Runner::Runner(const Pin<RunnerContext> runner_context) noexcept
    : runner_context_{runner_context} {}

auto
kero::Runner::Run() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = CreateServices(); res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  auto signal_service = runner_context_->GetService(kServiceKindSignal);
  auto is_interrupted = false;
  while (!is_interrupted) {
    if (signal_service) {
      auto signal =
          signal_service.Unwrap().As<SignalService>(kServiceKindSignal.id);
      if (signal) {
        is_interrupted = signal.Unwrap().IsInterrupted();
      }
    }

    UpdateServices();
  }

  DestroyServices();

  if (is_interrupted) {
    return ResultT::Err(Error::From(kInterrupted));
  }

  return OkVoid();
}

kero::ThreadRunner::ThreadRunner(Pin<Runner> runner) noexcept
    : runner_{runner} {}

auto
kero::ThreadRunner::Start() -> Result<Void> {
  if (thread_.joinable()) {
    return Result<Void>::Err(
        Json{}.Set("message", "thread already started").Take());
  }

  thread_ = std::thread{ThreadMain, runner_};
  return OkVoid();
}

auto
kero::ThreadRunner::Stop() -> Result<Void> {
  if (!thread_.joinable()) {
    return Result<Void>::Err(
        Json{}.Set("message", "thread not started").Take());
  }

  thread_.join();
  return OkVoid();
}

auto
kero::ThreadRunner::ThreadMain(Pin<Runner> runner) noexcept -> void {
  if (auto res = runner->Run()) {
    log::Info("Runner finished").Log();
  } else {
    log::Error("Runner failed").Data("error", res.TakeErr()).Log();
  }
}
