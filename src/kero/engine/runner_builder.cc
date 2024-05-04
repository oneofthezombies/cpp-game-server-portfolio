#include "runner_builder.h"

#include "kero/core/borrow.h"
#include "kero/engine/runner.h"

using namespace kero;

kero::RunnerBuilder::RunnerBuilder(const Borrow<EngineContext> engine_context,
                                   std::string&& runner_name) noexcept
    : engine_context_{engine_context}, runner_name_{std::move(runner_name)} {}

auto
kero::RunnerBuilder::AddServiceFactory(
    Own<ServiceFactory>&& service_factory) noexcept -> RunnerBuilder& {
  service_factories_.push_back(std::move(service_factory));
  return *this;
}

auto
kero::RunnerBuilder::AddServiceFactory(
    ServiceFactoryFn&& service_factory_fn) noexcept -> RunnerBuilder& {
  service_factories_.emplace_back(
      std::make_unique<ServiceFactoryFnImpl>(std::move(service_factory_fn)));
  return *this;
}

auto
kero::RunnerBuilder::BuildRunner() noexcept -> Result<Own<Runner>> {
  using ResultT = Result<Own<Runner>>;

  auto runner_context =
      std::make_unique<RunnerContext>(std::move(runner_name_));
  for (const auto& service_factory : service_factories_) {
    auto service_res = service_factory->Create(Borrow{runner_context});
    if (service_res.IsErr()) {
      return ResultT::Err(service_res.TakeErr());
    }

    auto service = service_res.TakeOk();
    if (auto res = runner_context->service_map_.AddService(std::move(service));
        res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }
  }

  auto runner = std::make_unique<Runner>(std::move(runner_context));
  return ResultT::Ok(std::move(runner));
}

auto
kero::RunnerBuilder::BuildThreadRunner() noexcept
    -> Result<Share<ThreadRunner>> {
  using ResultT = Result<Share<ThreadRunner>>;

  auto runner_res = BuildRunner();
  if (runner_res.IsErr()) {
    return ResultT::Err(runner_res.TakeErr());
  }

  auto runner = runner_res.TakeOk();
  return ResultT::Ok(std::make_shared<ThreadRunner>(std::move(runner)));
}
