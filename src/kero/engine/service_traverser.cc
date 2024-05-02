#include "service_traverser.h"

using namespace kero;

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

  return OkVoid();
}

auto
kero::ServiceTraverser::TraverseRecursive(const ServiceKind& service_kind,
                                          const OnVisit& on_visit) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (visit_map_.contains(service_kind.id)) {
    return OkVoid();
  }

  visit_map_.emplace(service_kind.id, service_kind.name);
  traversal_stack_.push_back(service_kind.id);

  const auto service_it = service_map_.find(service_kind.id);
  if (service_it == service_map_.end()) {
    return ResultT::Err(
        Json{}
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
                Json{}
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
            Json{}
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

  return OkVoid();
}
