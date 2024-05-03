#include "runner_builder.h"

#include "kero/engine/runner.h"

using namespace kero;

kero::RunnerBuilder::RunnerBuilder(const Borrowed<EngineContext> engine_context,
                                   std::string&& runner_name) noexcept
    : engine_context_{engine_context}, runner_name_{std::move(runner_name)} {}

auto
kero::RunnerBuilder::AddServiceFactory(
    Owned<ServiceFactory>&& service_factory) noexcept -> RunnerBuilder& {
  service_factories_.push_back(std::move(service_factory));
  return *this;
}

auto
kero::RunnerBuilder::BuildRunner() noexcept -> Result<Pin<Runner>> {
  using ResultT = Result<Pin<Runner>>;

  auto runner_context_output_res =
      engine_context_->pin_system.Create<RunnerContext>(
          []() { return Result<RunnerContext*>::Ok(new RunnerContext{}); });
  if (runner_context_output_res.IsErr()) {
    return ResultT::Err(runner_context_output_res.TakeErr());
  }

  auto runner_context_output = runner_context_output_res.TakeOk();
  auto runner_context = runner_context_output.pin;
  Defer destroy_runner_context{
      [destroyer = runner_context_output.destroyer] { destroyer(); }};

  for (const auto& service_factory : service_factories_) {
    auto service_res = service_factory->Create(runner_context);
    if (service_res.IsErr()) {
      return ResultT::Err(service_res.TakeErr());
    }

    auto service = service_res.TakeOk();
    if (auto res =
            runner_context->service_map_.AddService(std::move(service))) {
      return ResultT::Err(res.TakeErr());
    }
  }

  auto runner_output_res =
      engine_context_->pin_system.Create<Runner>([runner_context]() {
        return Result<Runner*>::Ok(new Runner{runner_context});
      });
  if (runner_output_res.IsErr()) {
    return ResultT::Err(runner_output_res.TakeErr());
  }

  auto runner_output = runner_output_res.TakeOk();
  auto runner = runner_output.pin;
  destroy_runner_context.Cancel();
  return ResultT::Ok(std::move(runner));
}

auto
kero::RunnerBuilder::BuildThreadRunner() noexcept -> Result<Pin<ThreadRunner>> {
  using ResultT = Result<Pin<ThreadRunner>>;

  auto runner_res = BuildRunner();
  if (runner_res.IsErr()) {
    return ResultT::Err(runner_res.TakeErr());
  }

  auto runner = runner_res.TakeOk();
  auto thread_runner_output_res =
      engine_context_->pin_system.Create<ThreadRunner>([runner]() {
        return Result<ThreadRunner*>::Ok(new ThreadRunner{runner});
      });
  if (thread_runner_output_res.IsErr()) {
    return ResultT::Err(thread_runner_output_res.TakeErr());
  }

  auto thread_runner_output = thread_runner_output_res.TakeOk();
  return ResultT::Ok(std::move(thread_runner_output.pin));
}
