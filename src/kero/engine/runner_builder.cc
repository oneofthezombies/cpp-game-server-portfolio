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
kero::RunnerBuilder::BuildRunner() noexcept -> Result<Pinned<Runner>> {
  using ResultT = Result<Pinned<Runner>>;

  auto runner_context_res =
      engine_context_->pinning_system.Register(new RunnerContext{});
  if (runner_context_res.IsErr()) {
    return ResultT::Err(runner_context_res.TakeErr());
  }

  auto runner_context = runner_context_res.TakeOk();
  Defer destroy_runner_context{[this, runner_context] {
    engine_context_->pinning_system.Destroy(runner_context.Get());
  }};

  for (const auto& service_factory : service_factories_) {
    auto res = service_factory->Create(runner_context);
    if (res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    auto service = res.TakeOk();
    if (auto res = runner_context->service_map.AddService(std::move(service))) {
      return ResultT::Err(res.TakeErr());
    }
  }

  auto runner_res =
      engine_context_->pinning_system.Register(new Runner{runner_context});
  if (runner_res.IsErr()) {
    return ResultT::Err(runner_res.TakeErr());
  }

  destroy_runner_context.Cancel();
  return ResultT::Ok(runner_res.TakeOk());
}

auto
kero::RunnerBuilder::BuildThreadRunner() noexcept
    -> Result<Pinned<ThreadRunner>> {
  using ResultT = Result<Pinned<ThreadRunner>>;

  auto runner_res = BuildRunner();
  if (runner_res.IsErr()) {
    return ResultT::Err(runner_res.TakeErr());
  }

  auto thread_runner_res = engine_context_->pinning_system.Register(
      new ThreadRunner{runner_res.TakeOk()});
  if (thread_runner_res.IsErr()) {
    return ResultT::Err(thread_runner_res.TakeErr());
  }

  return ResultT::Ok(thread_runner_res.TakeOk());
}
