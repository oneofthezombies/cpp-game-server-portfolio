#include "service_map.h"

#include "kero/core/borrow.h"
#include "kero/core/flat_json.h"
#include "kero/core/option.h"
#include "kero/core/result.h"
#include "kero/core/utils.h"
#include "kero/engine/service.h"
#include "kero/engine/service_read_only_map.h"
#include "kero/engine/service_traverser.h"

using namespace kero;

auto
kero::ServiceMap::InvokeCreate() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  ServiceTraverser traverser{*this};
  auto res = traverser.Traverse([this](Service& service) {
    using ResultT = Result<Void>;

    const auto& dependency_declarations = service.GetDependencyDeclarations();
    auto dependency_map_res = CreateReadOnly(dependency_declarations);
    if (dependency_map_res.IsErr()) {
      return ResultT::Err(dependency_map_res.TakeErr());
    }

    service.dependency_map_ = dependency_map_res.TakeOk();
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
kero::ServiceMap::InvokeUpdate() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  ServiceTraverser traverser{*this};
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
kero::ServiceMap::InvokeDestroy() noexcept -> void {
  for (auto& [_, service] : id_to_service_map_) {
    service->OnDestroy();
  }
  id_to_service_map_.clear();
  name_to_id_map_.clear();
}

auto
kero::ServiceMap::AddService(Own<Service>&& service) noexcept -> Result<Void> {
  const auto service_kind_id = service->GetKindId();
  auto found_service = id_to_service_map_.find(service_kind_id);
  if (found_service != id_to_service_map_.end()) {
    return Result<Void>::Err(FlatJson{}
                                 .Set("message", "service already exists")
                                 .Set("service_kind_id", service_kind_id)
                                 .Take());
  }

  const auto service_kind_name = service->GetKindName();
  auto found_id = name_to_id_map_.find(service_kind_name);
  if (found_id != name_to_id_map_.end()) {
    return Result<Void>::Err(FlatJson{}
                                 .Set("message", "service already exists")
                                 .Set("service_kind_name", service_kind_name)
                                 .Set("found_id", found_id->second)
                                 .Take());
  }

  id_to_service_map_.emplace(service_kind_id, std::move(service));
  name_to_id_map_.emplace(service_kind_name, service_kind_id);
  return Result<Void>::Ok(Void{});
}

auto
kero::ServiceMap::GetService(const ServiceKindId service_kind_id) const noexcept
    -> Option<Borrow<Service>> {
  auto it = id_to_service_map_.find(service_kind_id);
  if (it == id_to_service_map_.end()) {
    return None;
  }

  return Option<Borrow<Service>>::Some(Borrow<Service>{it->second});
}

auto
kero::ServiceMap::GetService(const ServiceKindName service_kind_name)
    const noexcept -> Option<Borrow<Service>> {
  auto it = name_to_id_map_.find(service_kind_name);
  if (it == name_to_id_map_.end()) {
    return None;
  }

  return GetService(it->second);
}

auto
kero::ServiceMap::HasService(const ServiceKindId service_kind_id) const noexcept
    -> bool {
  return id_to_service_map_.contains(service_kind_id);
}

auto
kero::ServiceMap::HasService(
    const ServiceKindName service_kind_name) const noexcept -> bool {
  return name_to_id_map_.contains(service_kind_name);
}

auto
kero::ServiceMap::FindNameById(const ServiceKindId service_kind_id)
    const noexcept -> Result<ServiceKindName> {
  using ResultT = Result<ServiceKindName>;

  auto it = std::find_if(name_to_id_map_.begin(),
                         name_to_id_map_.end(),
                         [service_kind_id](const auto& pair) {
                           return pair.second == service_kind_id;
                         });
  if (it == name_to_id_map_.end()) {
    return ResultT::Err(FlatJson{}
                            .Set("message", "service not found")
                            .Set("service_kind_id", service_kind_id)
                            .Take());
  }

  auto service_kind_name = it->first;
  return ResultT::Ok(std::move(service_kind_name));
}

auto
kero::ServiceMap::CreateReadOnly(
    const Service::DependencyDeclarations& dependency_declarations) noexcept
    -> Result<ServiceReadOnlyMap> {
  using ResultT = Result<ServiceReadOnlyMap>;

  ServiceReadOnlyMap::IdToServiceMap id_to_service_map;
  NameToIdMap name_to_id_map;
  for (const auto service_kind_id : dependency_declarations) {
    auto found_service = id_to_service_map_.find(service_kind_id);
    if (found_service == id_to_service_map_.end()) {
      return ResultT::Err(FlatJson{}
                              .Set("message", "service not found")
                              .Set("service_kind_id", service_kind_id)
                              .Take());
    }

    id_to_service_map.emplace(service_kind_id, found_service->second);
    name_to_id_map.emplace(found_service->second->GetKindName(),
                           service_kind_id);
  }

  return ResultT::Ok(ServiceReadOnlyMap{std::move(id_to_service_map),
                                        std::move(name_to_id_map)});
}
