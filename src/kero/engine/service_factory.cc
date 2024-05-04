#include "service_factory.h"

#include "kero/engine/service.h"

using namespace kero;

kero::ServiceFactoryFnImpl::ServiceFactoryFnImpl(
    ServiceFactoryFn&& service_factory_fn) noexcept
    : fn_{std::move(service_factory_fn)} {}

auto
kero::ServiceFactoryFnImpl::Create(
    const Pin<RunnerContext> runner_context) noexcept -> Result<Own<Service>> {
  return fn_(runner_context);
}
