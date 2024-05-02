#include "service_map.h"

#include "kero/core/json.h"
#include "kero/core/result.h"
#include "kero/engine/service.h"

using namespace kero;

auto
kero::ServiceMap::AddService(Owned<Service>&& service) noexcept
    -> Result<Void> {
  const auto kind = service->GetKind();
  auto found = service_map_.find(kind.id);
  if (found != service_map_.end()) {
    return Result<Void>::Err(Json{}
                                 .Set("message", "service already exists")
                                 .Set("kind_id", static_cast<double>(kind.id))
                                 .Set("kind_name", kind.name)
                                 .Take());
  }

  auto found_id = service_kind_id_map_.find(kind.name);
  if (found_id != service_kind_id_map_.end()) {
    return Result<Void>::Err(Json{}
                                 .Set("message", "service already exists")
                                 .Set("kind_id", static_cast<double>(kind.id))
                                 .Set("kind_name", kind.name)
                                 .Take());
  }

  service_map_.emplace(kind.id, std::move(service));
  service_kind_id_map_.emplace(kind.name, kind.id);
  return Result<Void>::Ok(Void{});
}

auto
kero::ServiceMap::GetService(const ServiceKind::Id service_kind_id)
    const noexcept -> OptionRef<Service&> {
  auto it = service_map_.find(service_kind_id);
  if (it == service_map_.end()) {
    return None;
  }

  return OptionRef<Service&>{*it->second};
}

auto
kero::ServiceMap::GetService(const ServiceKind::Name service_kind_name)
    const noexcept -> OptionRef<Service&> {
  auto it = service_kind_id_map_.find(service_kind_name);
  if (it == service_kind_id_map_.end()) {
    return None;
  }

  return GetService(it->second);
}

auto
kero::ServiceMap::HasService(
    const ServiceKind::Id service_kind_id) const noexcept -> bool {
  return service_map_.contains(service_kind_id);
}

auto
kero::ServiceMap::HasService(
    const ServiceKind::Name service_kind_name) const noexcept -> bool {
  return service_kind_id_map_.contains(service_kind_name);
}

auto
kero::ServiceMap::GetServiceMapRaw() const noexcept -> const ServiceMapRaw& {
  return service_map_;
}

auto
kero::ServiceMap::GetServiceMapRaw() noexcept -> ServiceMapRaw& {
  return service_map_;
}

auto
kero::ServiceMap::GetServiceKindIdMapRaw() const noexcept
    -> const ServiceKindIdMapRaw& {
  return service_kind_id_map_;
}

auto
kero::ServiceMap::GetServiceKindIdMapRaw() noexcept -> ServiceKindIdMapRaw& {
  return service_kind_id_map_;
}
