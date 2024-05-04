#ifndef KERO_ENGINE_SERVICE_TRAVERSER_H
#define KERO_ENGINE_SERVICE_TRAVERSER_H

#include <functional>
#include <unordered_set>

#include "kero/core/common.h"
#include "kero/core/result.h"
#include "kero/engine/service_kind.h"
#include "kero/engine/service_map.h"

namespace kero {

class ServiceTraverser {
 public:
  using OnVisit = std::function<Result<Void>(Service& service)>;
  using VisitSet = std::unordered_set<ServiceKindId>;
  using TraversalStack = std::vector<ServiceKindId>;

  explicit ServiceTraverser(const ServiceMap& service_map) noexcept;
  ~ServiceTraverser() noexcept = default;
  CLASS_KIND_PINNABLE(ServiceTraverser);

  [[nodiscard]] auto
  Traverse(const OnVisit& on_visit) noexcept -> Result<Void>;

 private:
  [[nodiscard]] auto
  TraverseRecursive(const ServiceKindId service_kind_id,
                    const OnVisit& on_visit) noexcept -> Result<Void>;

  const ServiceMap& service_map_;
  VisitSet visit_set_;
  TraversalStack traversal_stack_;
};

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_TRAVERSER_H