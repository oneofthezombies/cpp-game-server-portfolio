#include "runner.h"

#include "kero/core/borrow.h"
#include "kero/core/utils.h"
#include "kero/engine/common.h"
#include "kero/engine/runner_builder.h"
#include "kero/engine/runner_context.h"
#include "kero/engine/service.h"
#include "kero/engine/service_map.h"
#include "kero/engine/signal_service.h"
#include "kero/log/log_builder.h"

using namespace kero;

kero::Runner::Runner(Own<RunnerContext>&& runner_context) noexcept
    : runner_context_{std::move(runner_context)} {}

auto
kero::Runner::Run() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = runner_context_->service_map_.InvokeCreate(); res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  auto signal_service =
      runner_context_->service_map_.GetService<SignalService>();
  auto is_interrupted = false;
  while (!is_interrupted) {
    if (signal_service) {
      is_interrupted = signal_service.Unwrap()->IsInterrupted();
    }

    if (auto res = runner_context_->service_map_.InvokeUpdate(); res.IsErr()) {
      log::Error("service update failed").Data("error", res.TakeErr()).Log();
    }
  }

  runner_context_->service_map_.InvokeDestroy();

  if (is_interrupted) {
    return ResultT::Err(Error::From(kInterrupted));
  }

  return OkVoid();
}

kero::ThreadRunner::ThreadRunner(Own<Runner>&& runner) noexcept
    : runner_{std::move(runner)} {}

auto
kero::ThreadRunner::Start() -> Result<Void> {
  if (thread_.joinable()) {
    return Result<Void>::Err(
        FlatJson{}.Set("message", "thread already started").Take());
  }

  thread_ = std::thread{ThreadMain, Borrow{runner_}};
  return OkVoid();
}

auto
kero::ThreadRunner::Stop() -> Result<Void> {
  if (!thread_.joinable()) {
    return Result<Void>::Err(
        FlatJson{}.Set("message", "thread not started").Take());
  }

  thread_.join();
  return OkVoid();
}

auto
kero::ThreadRunner::ThreadMain(const Borrow<Runner> runner) noexcept -> void {
  if (auto res = runner->Run()) {
    log::Info("Runner finished").Log();
  } else {
    log::Error("Runner failed").Data("error", res.TakeErr()).Log();
  }
}
