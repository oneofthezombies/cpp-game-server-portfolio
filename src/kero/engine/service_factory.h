#ifndef KERO_ENGINE_SERVICE_FACTORY_H
#define KERO_ENGINE_SERVICE_FACTORY_H

#include "kero/core/borrow.h"
#include "kero/core/common.h"
#include "kero/core/result.h"
#include "kero/engine/pin.h"
#include "kero/engine/service_kind.h"

namespace kero {

class Service;
class RunnerContext;

class ServiceFactory {
 public:
  explicit ServiceFactory() noexcept = default;
  virtual ~ServiceFactory() noexcept = default;
  KERO_CLASS_KIND_MOVABLE(ServiceFactory);

  [[nodiscard]] virtual auto
  Create(const Borrow<RunnerContext> runner_context) noexcept
      -> Result<Own<Service>> = 0;
};

using ServiceFactoryFn =
    std::function<Result<Own<Service>>(const Borrow<RunnerContext>)>;

class ServiceFactoryFnImpl final : public ServiceFactory {
 public:
  explicit ServiceFactoryFnImpl(ServiceFactoryFn&& service_factory_fn) noexcept;

  virtual ~ServiceFactoryFnImpl() noexcept override = default;
  KERO_CLASS_KIND_MOVABLE(ServiceFactoryFnImpl);

  [[nodiscard]] virtual auto
  Create(const Borrow<RunnerContext> runner_context) noexcept
      -> Result<Own<Service>> override;

 private:
  ServiceFactoryFn fn_;
};

template <IsServiceKind T>
class DefaultServiceFactory final : public ServiceFactory {
 public:
  explicit DefaultServiceFactory() noexcept = default;
  virtual ~DefaultServiceFactory() noexcept override = default;
  KERO_CLASS_KIND_MOVABLE(DefaultServiceFactory);

  [[nodiscard]] virtual auto
  Create(const Borrow<RunnerContext> runner_context) noexcept
      -> Result<Own<Service>> override {
    return Result<Own<Service>>::Ok(std::make_unique<T>(runner_context));
  };
};

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_FACTORY_H
