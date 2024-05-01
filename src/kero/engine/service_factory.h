#ifndef KERO_ENGINE_SERVICE_FACTORY_H
#define KERO_ENGINE_SERVICE_FACTORY_H

#include <memory>

#include "kero/core/common.h"

namespace kero {

class Service;

class ServiceFactory {
 public:
  explicit ServiceFactory() noexcept = default;
  virtual ~ServiceFactory() noexcept = default;
  CLASS_KIND_MOVABLE(ServiceFactory);

  [[nodiscard]] virtual auto
  Create() noexcept -> Owned<Service> = 0;
};

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_FACTORY_H
