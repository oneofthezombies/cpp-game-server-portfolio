#include "service.h"

#include "kero/engine/runner.h"

using namespace kero;

kero::Service::Service(const Pin<RunnerContext> runner_context,
                       const Kind& kind,
                       Dependencies&& dependencies) noexcept
    : runner_context_{runner_context},
      kind_{kind},
      dependencies_{std::move(dependencies)} {}

auto
kero::Service::GetKind() const noexcept -> const Kind& {
  return kind_;
}

auto
kero::Service::GetDependencies() const noexcept -> const Dependencies& {
  return dependencies_;
}

auto
kero::Service::Is(const Kind::Id kind_id) const noexcept -> bool {
  return kind_.id == kind_id;
}

auto
kero::Service::Is(const Kind::Name& kind_name) const noexcept -> bool {
  return kind_.name == kind_name;
}

auto
kero::Service::SubscribeEvent(const std::string& event) -> Result<Void> {
  return runner_context_.Unwrap().SubscribeEvent(event, kind_);
}

auto
kero::Service::UnsubscribeEvent(const std::string& event) -> Result<Void> {
  return runner_context_.Unwrap().UnsubscribeEvent(event, kind_);
}

auto
kero::Service::InvokeEvent(const std::string& event, const Dict& data) noexcept
    -> void {
  runner_context_.Unwrap().InvokeEvent(event, data);
}

auto
kero::Service::OnCreate() noexcept -> Result<Void> {
  return Result<Void>::Ok(Void{});
}

auto
kero::Service::OnDestroy() noexcept -> void {
  // noop
}

auto
kero::Service::OnUpdate() noexcept -> void {
  // noop
}

auto
kero::Service::OnEvent(const std::string& event, const Dict& data) noexcept
    -> void {
  // noop
}

auto
kero::ServiceMap::AddService(ServicePtr&& service) noexcept -> Result<Void> {
  const auto kind = service->GetKind();
  auto found = service_map_.find(kind.id);
  if (found != service_map_.end()) {
    return Result<Void>::Err(Dict{}
                                 .Set("message", "service already exists")
                                 .Set("kind_id", static_cast<double>(kind.id))
                                 .Set("kind_name", kind.name)
                                 .Take());
  }

  auto found_id = service_kind_id_map_.find(kind.name);
  if (found_id != service_kind_id_map_.end()) {
    return Result<Void>::Err(Dict{}
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
kero::ServiceMap::GetService(const Service::Kind::Id service_kind_id)
    const noexcept -> OptionRef<Service&> {
  auto it = service_map_.find(service_kind_id);
  if (it == service_map_.end()) {
    return None;
  }

  return OptionRef<Service&>{*it->second};
}

auto
kero::ServiceMap::GetService(const Service::Kind::Name service_kind_name)
    const noexcept -> OptionRef<Service&> {
  auto it = service_kind_id_map_.find(service_kind_name);
  if (it == service_kind_id_map_.end()) {
    return None;
  }

  return GetService(it->second);
}

auto
kero::ServiceMap::HasService(
    const Service::Kind::Id service_kind_id) const noexcept -> bool {
  return service_map_.contains(service_kind_id);
}

auto
kero::ServiceMap::HasService(
    const Service::Kind::Name service_kind_name) const noexcept -> bool {
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

kero::ServiceTraverser::ServiceTraverser(
    const ServiceMap::ServiceMapRaw& service_map) noexcept
    : service_map_{service_map} {}

auto
kero::ServiceTraverser::Traverse(const OnVisit& on_visit) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  visit_map_.clear();
  traversal_stack_.clear();

  for (const auto& [_, service] : service_map_) {
    auto result = TraverseRecursive(service->GetKind(), on_visit);
    if (result.IsErr()) {
      return ResultT::Err(result.TakeErr());
    }
  }

  return OkVoid;
}

auto
kero::ServiceTraverser::TraverseRecursive(const Service::Kind& service_kind,
                                          const OnVisit& on_visit) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (visit_map_.contains(service_kind.id)) {
    return OkVoid;
  }

  visit_map_.emplace(service_kind.id, service_kind.name);
  traversal_stack_.push_back(service_kind.id);

  const auto service_it = service_map_.find(service_kind.id);
  if (service_it == service_map_.end()) {
    return ResultT::Err(
        Dict{}
            .Set("message", "service not found in service map")
            .Set("service_kind_id", static_cast<double>(service_kind.id))
            .Take());
  }

  auto& service = *service_it->second;
  const auto& dependencies = service.GetDependencies();
  for (const auto& dependency : dependencies) {
    auto visit_it = visit_map_.find(dependency.id);
    if (visit_it != visit_map_.end()) {
      auto found = std::find(traversal_stack_.begin(),
                             traversal_stack_.end(),
                             dependency.id);
      if (found != traversal_stack_.end()) {
        std::string circular_dependencies = dependency.name + " -> ";
        for (auto it = std::next(found); it != traversal_stack_.end(); ++it) {
          auto circular_it = visit_map_.find(*it);
          if (circular_it == visit_map_.end()) {
            return ResultT::Err(
                Dict{}
                    .Set("message", "circular dependency must be found")
                    .Set("service_kind_id", static_cast<double>(*it))
                    .Take());
          }

          circular_dependencies += circular_it->second;
          if (std::next(it) != traversal_stack_.end()) {
            circular_dependencies += " -> ";
          }
        }

        return ResultT::Err(
            Dict{}
                .Set("message", "circular dependencies found")
                .Set("circular_dependencies", std::move(circular_dependencies))
                .Take());
      }
    } else {
      auto result = TraverseRecursive(dependency, on_visit);
      if (result.IsErr()) {
        return ResultT::Err(result.TakeErr());
      }
    }
  }

  traversal_stack_.pop_back();
  auto res = on_visit(service);
  if (res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  return OkVoid;
}
