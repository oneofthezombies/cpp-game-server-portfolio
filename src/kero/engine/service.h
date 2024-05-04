#ifndef KERO_ENGINE_SERVICE_H
#define KERO_ENGINE_SERVICE_H

#include <string>

#include "kero/core/borrow.h"
#include "kero/core/common.h"
#include "kero/core/flat_json.h"
#include "kero/core/result.h"
#include "kero/engine/pin.h"
#include "kero/engine/service_kind.h"
#include "kero/engine/service_read_only_map.h"

namespace kero {

class RunnerContext;

class Service {
 public:
  using DependencyDeclarations = std::vector<ServiceKindId>;

  explicit Service(const Borrow<RunnerContext> runner_context,
                   DependencyDeclarations&& dependency_declarations) noexcept;

  virtual ~Service() noexcept = default;
  KERO_CLASS_KIND_MOVABLE(Service);

  [[nodiscard]] virtual auto
  GetKindId() const noexcept -> ServiceKindId = 0;

  [[nodiscard]] virtual auto
  GetKindName() const noexcept -> ServiceKindName = 0;

  [[nodiscard]] auto
  GetDependencyDeclarations() const noexcept -> const DependencyDeclarations&;

  [[nodiscard]] auto
  GetDependency(const ServiceKindId kind_id) noexcept -> Borrow<Service>;

  [[nodiscard]] auto
  GetDependency(const ServiceKindName kind_name) noexcept -> Borrow<Service>;

  template <IsServiceKind T>
  [[nodiscard]] auto
  GetDependency() noexcept -> Borrow<T> {
    return dependency_map_.GetService<T>();
  }

  [[nodiscard]] auto
  SubscribeEvent(const std::string& event) -> Result<Void>;

  [[nodiscard]] auto
  UnsubscribeEvent(const std::string& event) -> Result<Void>;

  auto
  InvokeEvent(const std::string& event,
              const FlatJson& data) noexcept -> Result<Void>;

  /**
   * Default implementation of the `OnCreate` method is noop.
   */
  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void>;

  /**
   * Default implementation of the `OnDestroy` method is noop.
   */
  virtual auto
  OnDestroy() noexcept -> void;

  /**
   * Default implementation of the `OnUpdate` method is noop.
   */
  virtual auto
  OnUpdate() noexcept -> void;

  /**
   * Default implementation of the `OnEvent` method is noop.
   */
  virtual auto
  OnEvent(const std::string& event, const FlatJson& data) noexcept -> void;

 protected:
  DependencyDeclarations dependency_declarations_;

 private:
  ServiceReadOnlyMap dependency_map_{};
  Borrow<RunnerContext> runner_context_;

  friend class ServiceMap;
};

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_H
