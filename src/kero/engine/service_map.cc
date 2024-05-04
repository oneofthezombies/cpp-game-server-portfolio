#include "service_map.h"

#include "kero/core/json.h"
#include "kero/core/option.h"
#include "kero/core/result.h"
#include "kero/core/utils.h"
#include "kero/engine/service.h"
#include "kero/engine/service_traverser.h"
#include "service_read_only_map.h"

using namespace kero;

auto
kero::ServiceMap::InvokeCreate() noexcept {
  using ResultT = Result<Void>;

  ServiceTraverser traverser{id_to_service_map_};
  auto res = traverser.Traverse([this](Service& service) {
    using ResultT = Result<Void>;

    const auto& dependency_declarations = service.GetDependencyDeclarations();
    auto dependency_map = CreateReadOnly(dependency_declarations);
    service.dependency_map_ = std::move(dependency_map);
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
  using ResultT = Result<Void>;

  ServiceTraverser traverser{id_to_service_map_};
  auto res = traverser.Traverse([](Service& service) {
    service.OnUpdate();

    return OkVoid();
  });

  if (res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  return OkVoid();
}

auto
kero::ServiceMap::InvokeDestroy() noexcept {
  for (auto& [_, service] : id_to_service_map_) {
    service->OnDestroy();
  }
  id_to_service_map_.clear();
  name_to_id_map_.clear();
}

auto
kero::ServiceMap::AddService(Own<Service>&& service) noexcept -> Result<Void> {
  const auto kind = service->GetKind();
  auto found = id_to_service_map_.find(kind.id);
  if (found != id_to_service_map_.end()) {
    return Result<Void>::Err(Json{}
                                 .Set("message", "service already exists")
                                 .Set("kind_id", static_cast<double>(kind.id))
                                 .Set("kind_name", kind.name)
                                 .Take());
  }

  auto found_id = name_to_id_map_.find(kind.name);
  if (found_id != name_to_id_map_.end()) {
    return Result<Void>::Err(Json{}
                                 .Set("message", "service already exists")
                                 .Set("kind_id", static_cast<double>(kind.id))
                                 .Set("kind_name", kind.name)
                                 .Take());
  }

  id_to_service_map_.emplace(kind.id, std::move(service));
  name_to_id_map_.emplace(kind.name, kind.id);
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
  auto it = id_to_service_map_.find(service_kind_id);
  if (it == id_to_service_map_.end()) {
    return None;
  }

  return OptionRef<Service&>{*it->second};
}

auto
kero::ServiceMap::GetService(const ServiceKind::Name& service_kind_name)
    const noexcept -> OptionRef<Service&> {
  auto it = name_to_id_map_.find(service_kind_name);
  if (it == name_to_id_map_.end()) {
    return None;
  }

  return GetService(it->second);
}

auto
kero::ServiceMap::HasService(
    const ServiceKind::Id service_kind_id) const noexcept -> bool {
  return id_to_service_map_.contains(service_kind_id);
}

auto
kero::ServiceMap::HasService(
    const ServiceKind::Name service_kind_name) const noexcept -> bool {
  return name_to_id_map_.contains(service_kind_name);
}

auto
kero::ServiceMap::CreateReadOnly(
    const Service::DependencyDeclarations& dependency_declarations) noexcept
    -> ServiceReadOnlyMap {
  ServiceReadOnlyMap::IdToServiceMap id_to_service_map;
  NameToIdMap name_to_id_map;
  for (auto& [id, service] : id_to_service_map_) {
    auto found = std::find_if(
        dependency_declarations.begin(),
        dependency_declarations.end(),
        [id](const ServiceKind& declaration) { return declaration.id == id; });
    if (found == dependency_declarations.end()) {
      continue;
    }

    id_to_service_map.emplace(id, service);
    name_to_id_map.emplace(service->GetKind().name, id);
  }

  return ServiceReadOnlyMap{std::move(id_to_service_map),
                            std::move(name_to_id_map)};
}
