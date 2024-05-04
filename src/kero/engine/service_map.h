#ifndef KERO_ENGINE_SERVICE_MAP_H
#define KERO_ENGINE_SERVICE_MAP_H

#include <unordered_map>

#include "kero/core/common.h"
#include "kero/core/option.h"
#include "kero/core/result.h"
#include "kero/engine/service.h"
#include "kero/engine/service_read_only_map.h"

namespace kero {

class ServiceMap {
 public:
  using IdToServiceMap = std::unordered_map<ServiceKindId, Own<Service>>;
  using NameToIdMap = std::unordered_map<ServiceKindName, ServiceKindId>;

  explicit ServiceMap() noexcept = default;
  ~ServiceMap() noexcept = default;
  CLASS_KIND_MOVABLE(ServiceMap);

  [[nodiscard]] auto
  InvokeCreate() noexcept;

  [[nodiscard]] auto
  InvokeUpdate() noexcept;

  [[nodiscard]] auto
  InvokeDestroy() noexcept;

  [[nodiscard]] auto
  AddService(Own<Service>&& service) noexcept -> Result<Void>;

  [[nodiscard]] auto
  GetService(const ServiceKindId service_kind_id) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  GetService(const ServiceKindName service_kind_name) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  HasService(const ServiceKindId service_kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  HasService(const ServiceKindName service_kind_name) const noexcept -> bool;

  [[nodiscard]] auto
  CreateReadOnly(
      const Service::DependencyDeclarations& dependency_declarations) noexcept
      -> ServiceReadOnlyMap;

 private:
  IdToServiceMap id_to_service_map_;
  NameToIdMap name_to_id_map_;

  friend class ServiceTraverser;
};

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_MAP_H
