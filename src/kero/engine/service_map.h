#ifndef KERO_ENGINE_SERVICE_MAP_H
#define KERO_ENGINE_SERVICE_MAP_H

#include <unordered_map>

#include "kero/core/common.h"
#include "kero/core/option.h"
#include "kero/core/result.h"
#include "kero/engine/service.h"
#include "kero/engine/service_read_only_map.h"
#include "service_kind.h"

namespace kero {

class ServiceMap {
 public:
  using IdToServiceMap = std::unordered_map<ServiceKindId, Own<Service>>;
  using NameToIdMap = std::unordered_map<ServiceKindName, ServiceKindId>;

  explicit ServiceMap() noexcept = default;
  ~ServiceMap() noexcept = default;
  CLASS_KIND_MOVABLE(ServiceMap);

  [[nodiscard]] auto
  InvokeCreate() noexcept -> Result<Void>;

  [[nodiscard]] auto
  InvokeUpdate() noexcept -> Result<Void>;

  auto
  InvokeDestroy() noexcept -> void;

  [[nodiscard]] auto
  AddService(Own<Service>&& service) noexcept -> Result<Void>;

  [[nodiscard]] auto
  GetService(const ServiceKindId service_kind_id) const noexcept
      -> Option<Borrow<Service>>;

  [[nodiscard]] auto
  GetService(const ServiceKindName service_kind_name) const noexcept
      -> Option<Borrow<Service>>;

  template <IsServiceKind T>
  [[nodiscard]] auto
  GetService() const noexcept -> Option<Borrow<T>> {
    const auto service_kind_id = T::kKindId;
    auto it = id_to_service_map_.find(service_kind_id);
    if (it == id_to_service_map_.end()) {
      return None;
    }

    auto& ptr = it->second;
    return Option<Borrow<T>>::Some(Borrow<T>{static_cast<T*>(ptr.get())});
  }

  [[nodiscard]] auto
  HasService(const ServiceKindId service_kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  HasService(const ServiceKindName service_kind_name) const noexcept -> bool;

  [[nodiscard]] auto
  FindNameById(const ServiceKindId service_kind_id) const noexcept
      -> Result<ServiceKindName>;

  [[nodiscard]] auto
  CreateReadOnly(
      const Service::DependencyDeclarations& dependency_declarations) noexcept
      -> Result<ServiceReadOnlyMap>;

 private:
  IdToServiceMap id_to_service_map_;
  NameToIdMap name_to_id_map_;

  friend class ServiceTraverser;
};

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_MAP_H
