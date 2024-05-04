#include "service_traverser.h"

#include "kero/core/utils.h"

using namespace kero;

kero::ServiceTraverser::ServiceTraverser(const ServiceMap& service_map) noexcept
    : service_map_{service_map} {}

auto
kero::ServiceTraverser::Traverse(const OnVisit& on_visit) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  visit_set_.clear();
  traversal_stack_.clear();

  for (const auto& [_, service] : service_map_.id_to_service_map_) {
    auto result = TraverseRecursive(service->GetKindId(), on_visit);
    if (result.IsErr()) {
      return ResultT::Err(result.TakeErr());
    }
  }

  return OkVoid();
}

auto
kero::ServiceTraverser::TraverseRecursive(const ServiceKindId service_kind_id,
                                          const OnVisit& on_visit) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (visit_set_.contains(service_kind_id)) {
    return OkVoid();
  }

  visit_set_.insert(service_kind_id);
  traversal_stack_.push_back(service_kind_id);

  const auto service_it = service_map_.id_to_service_map_.find(service_kind_id);
  if (service_it == service_map_.id_to_service_map_.end()) {
    return ResultT::Err(FlatJson{}
                            .Set("message", "service not found in service map")
                            .Set("service_kind_id", service_kind_id)
                            .Take());
  }

  auto& service = *service_it->second;
  const auto& dependency_declarations = service.GetDependencyDeclarations();
  for (const auto& dependency : dependency_declarations) {
    auto visit_it = visit_set_.find(dependency);
    if (visit_it != visit_set_.end()) {
      auto found = std::find(traversal_stack_.begin(),
                             traversal_stack_.end(),
                             dependency);
      if (found != traversal_stack_.end()) {
        auto dependency_name_res = service_map_.FindNameById(dependency);
        if (dependency_name_res.IsErr()) {
          return ResultT::Err(dependency_name_res.TakeErr());
        }

        const auto dependency_name = dependency_name_res.TakeOk();
        std::string circular_dependencies =
            std::string{dependency_name} + " -> ";
        for (auto traversal_it = std::next(found);
             traversal_it != traversal_stack_.end();
             ++traversal_it) {
          auto circular_it = visit_set_.find(*traversal_it);
          if (circular_it == visit_set_.end()) {
            return ResultT::Err(
                FlatJson{}
                    .Set("message", "circular dependency must be found")
                    .Set("service_kind_id", *traversal_it)
                    .Take());
          }

          const auto circular_dependency = *circular_it;
          auto circular_name_res =
              service_map_.FindNameById(circular_dependency);
          if (circular_name_res.IsErr()) {
            return ResultT::Err(circular_name_res.TakeErr());
          }

          const auto circular_name = circular_name_res.TakeOk();
          circular_dependencies += circular_name;
          if (std::next(traversal_it) != traversal_stack_.end()) {
            circular_dependencies += " -> ";
          }
        }

        return ResultT::Err(
            FlatJson{}
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
