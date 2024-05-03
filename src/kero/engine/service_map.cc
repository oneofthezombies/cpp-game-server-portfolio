#include "service_map.h"

#include "kero/core/json.h"
#include "kero/core/result.h"
#include "kero/core/utils.h"
#include "kero/engine/service.h"
#include "kero/engine/service_traverser.h"

using namespace kero;

auto
kero::ServiceMap::InvokeCreate() noexcept {
  using ResultT = Result<Void>;

  ServiceTraverser traverser{service_map_};
  auto res = traverser.Traverse([](Service& service) {
    using ResultT = Result<Void>;

    if (auto res = service.OnCreate(); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    return OkVoid();
  });

  if (res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  return OkVoid();
}

auto
kero::ServiceMap::InvokeUpdate() noexcept {
  for (auto& [_, service] : service_map_) {
    service->OnUpdate();
  }
}

auto
kero::ServiceMap::InvokeDestroy() noexcept {
  for (auto& [_, service] : service_map_) {
    service->OnDestroy();
  }
  service_map_.clear();
  service_kind_id_map_.clear();
}

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
kero::ServiceMap::GetService(const ServiceKind& kind) const noexcept
    -> OptionRef<Service&> {
  return GetService(kind.id);
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
kero::ServiceMap::GetService(const ServiceKind::Name& service_kind_name)
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
