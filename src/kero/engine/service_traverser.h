#ifndef KERO_ENGINE_SERVICE_TRAVERSER_H
#define KERO_ENGINE_SERVICE_TRAVERSER_H

#include <functional>

#include "kero/core/common.h"
#include "kero/core/result.h"
#include "kero/engine/service_kind.h"
#include "kero/engine/service_map.h"

namespace kero {

class ServiceTraverser {
 public:
  using OnVisit = std::function<Result<Void>(Service& service)>;

  /**
   * The "kind_id" is the key and the "kind_name" is the value.
   * If "kind_id" exists, it means visited. Otherwise, it means not visited.
   */
  using VisitMap = std::unordered_map<ServiceKindId, ServiceKindName>;
  using TraversalStack = std::vector<ServiceKindId>;

  explicit ServiceTraverser(
      const ServiceMap::IdToServiceMap& id_to_service_map) noexcept;
  ~ServiceTraverser() noexcept = default;
  CLASS_KIND_PINNABLE(ServiceTraverser);

  [[nodiscard]] auto
  Traverse(const OnVisit& on_visit) noexcept -> Result<Void>;

 private:
  [[nodiscard]] auto
  TraverseRecursive(const ServiceKindId service_kind_id,
                    const OnVisit& on_visit) noexcept -> Result<Void>;

  const ServiceMap::IdToServiceMap& id_to_service_map_;
  VisitMap visit_map_;
  TraversalStack traversal_stack_;
};

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_TRAVERSER_H