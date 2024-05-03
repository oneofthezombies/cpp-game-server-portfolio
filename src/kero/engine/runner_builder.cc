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

  auto runner_context_res = engine_context_->pin_system.Create<RunnerContext>(
      []() { return Result<RunnerContext*>::Ok(new RunnerContext{}); });
  if (runner_context_res.IsErr()) {
    return ResultT::Err(runner_context_res.TakeErr());
  }

  auto pin_res_runner_context = runner_context_res.TakeOk();
  auto pin_runner_context = pin_res_runner_context.pin;
  Defer destroy_runner_context{
      [destroyer = pin_res_runner_context.destroyer] { destroyer(); }};

  for (const auto& service_factory : service_factories_) {
    auto service_res = service_factory->Create(pin_runner_context);
    if (service_res.IsErr()) {
      return ResultT::Err(service_res.TakeErr());
    }

    auto service = service_res.TakeOk();
    if (auto res =
            pin_runner_context->service_map.AddService(std::move(service))) {
      return ResultT::Err(res.TakeErr());
    }
  }

  auto runner_res =
      engine_context_->pin_system.Register(new Runner{runner_context});
  if (runner_res.IsErr()) {
    return ResultT::Err(runner_res.TakeErr());
  }

  destroy_runner_context.Cancel();
  return ResultT::Ok(runner_res.TakeOk());
}

auto
kero::RunnerBuilder::BuildThreadRunner() noexcept -> Result<Pin<ThreadRunner>> {
  using ResultT = Result<Pin<ThreadRunner>>;

  auto runner_res = BuildRunner();
  if (runner_res.IsErr()) {
    return ResultT::Err(runner_res.TakeErr());
  }

  auto thread_runner_res = engine_context_->pin_system.Register(
      new ThreadRunner{runner_res.TakeOk()});
  if (thread_runner_res.IsErr()) {
    return ResultT::Err(thread_runner_res.TakeErr());
  }

  return ResultT::Ok(thread_runner_res.TakeOk());
}
