#ifndef KERO_ENGINE_SERVICE_READ_ONLY_MAP_H
#define KERO_ENGINE_SERVICE_READ_ONLY_MAP_H

#include <unordered_map>

#include "kero/core/borrow.h"
#include "kero/engine/service_kind.h"

namespace kero {

class ServiceReadOnlyMap {
 public:
  using IdToServiceMap = std::unordered_map<ServiceKindId, Borrow<Service>>;
  using NameToIdMap = std::unordered_map<ServiceKindName, ServiceKindId>;

  explicit ServiceReadOnlyMap() noexcept = default;
  ~ServiceReadOnlyMap() noexcept = default;
  CLASS_KIND_MOVABLE(ServiceReadOnlyMap);

  [[nodiscard]] auto
  GetService(const ServiceKindId service_kind_id) const noexcept
      -> Borrow<Service>;

  [[nodiscard]] auto
  GetService(const ServiceKindName service_kind_name) const noexcept
      -> Borrow<Service>;

  template <IsServiceKind T>
  [[nodiscard]] auto
  GetService() const noexcept -> Borrow<T> {
    const auto service_kind_id = T::GetKindId();
    auto it = id_to_service_map_.find(service_kind_id);
    auto& ptr = it->second;
    return Borrow<T>{static_cast<T*>(ptr.Get())};
  }

 private:
  explicit ServiceReadOnlyMap(IdToServiceMap&& id_to_service_map,
                              NameToIdMap&& name_to_id_map) noexcept;

  IdToServiceMap id_to_service_map_;
  NameToIdMap name_to_id_map_;

  friend class ServiceMap;
};

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_READ_ONLY_MAP_H
