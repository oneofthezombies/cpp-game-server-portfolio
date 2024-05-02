#ifndef KERO_ENGINE_SERVICE_FACTORY_H
#define KERO_ENGINE_SERVICE_FACTORY_H

#include "kero/core/common.h"
#include "kero/core/result.h"
#include "kero/engine/pinned.h"

namespace kero {

class Service;
class RunnerContext;

class ServiceFactory {
 public:
  explicit ServiceFactory() noexcept = default;
  virtual ~ServiceFactory() noexcept = default;
  CLASS_KIND_MOVABLE(ServiceFactory);

  [[nodiscard]] virtual auto
  Create(const Pinned<RunnerContext> runner_context) noexcept
      -> Result<Owned<Service>> = 0;
};

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_FACTORY_H
