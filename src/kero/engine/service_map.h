#ifndef KERO_ENGINE_SERVICE_MAP_H
#define KERO_ENGINE_SERVICE_MAP_H

#include <unordered_map>

#include "kero/core/common.h"
#include "kero/core/option.h"
#include "kero/core/result.h"
#include "kero/engine/service.h"

namespace kero {

class ServiceMap {
 public:
  using ServiceMapRaw = std::unordered_map<ServiceKind::Id, Owned<Service>>;
  using ServiceKindIdMapRaw =
      std::unordered_map<ServiceKind::Name, ServiceKind::Id>;

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
  AddService(Owned<Service>&& service) noexcept -> Result<Void>;

  [[nodiscard]] auto
  GetService(const ServiceKind& kind) const noexcept -> OptionRef<Service&>;

  [[nodiscard]] auto
  GetService(const ServiceKind::Id service_kind_id) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  GetService(const ServiceKind::Name& service_kind_name) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  HasService(const ServiceKind::Id service_kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  HasService(const ServiceKind::Name service_kind_name) const noexcept -> bool;

 private:
  ServiceMapRaw service_map_;
  ServiceKindIdMapRaw service_kind_id_map_;

  friend class ServiceTraverser;
};

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_MAP_H
