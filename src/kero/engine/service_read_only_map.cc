#include "service_read_only_map.h"

using namespace kero;

kero::ServiceReadOnlyMap::ServiceReadOnlyMap(
    IdToServiceMap&& id_to_service_map, NameToIdMap&& name_to_id_map) noexcept
    : id_to_service_map_{id_to_service_map}, name_to_id_map_{name_to_id_map} {}

auto
kero::ServiceReadOnlyMap::GetService(
    const ServiceKindId service_kind_id) const noexcept -> Borrow<Service> {
  auto it = id_to_service_map_.find(service_kind_id);
  return it->second;
}

auto
kero::ServiceReadOnlyMap::GetService(
    const ServiceKindName service_kind_name) const noexcept -> Borrow<Service> {
  auto it = name_to_id_map_.find(service_kind_name);
  return GetService(it->second);
}
