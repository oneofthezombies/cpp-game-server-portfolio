#ifndef KERO_ENGINE_SERVICE_H
#define KERO_ENGINE_SERVICE_H

#include <string>
#include <unordered_map>

#include "kero/core/common.h"
#include "kero/core/dict.h"
#include "kero/core/result.h"
#include "kero/engine/pin_object_system.h"

namespace kero {

class RunnerContext;
class Service;

template <typename T>
concept IsServiceKind = std::is_base_of_v<Service, T>;

class Service {
 public:
  struct Kind {
    using Id = int64_t;
    using Name = std::string;

    Id id;
    Name name;
  };

  using Dependencies = std::vector<Kind>;

  explicit Service(const Pin<RunnerContext> runner_context,
                   const Kind& kind,
                   Dependencies&& dependencies) noexcept;

  virtual ~Service() noexcept = default;
  CLASS_KIND_MOVABLE(Service);

  [[nodiscard]] auto
  GetKind() const noexcept -> const Kind&;

  [[nodiscard]] auto
  GetDependencies() const noexcept -> const Dependencies&;

  [[nodiscard]] auto
  GetRunnerContext() noexcept -> RunnerContext&;

  [[nodiscard]] auto
  Is(const Kind::Id kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  Is(const Kind::Name& kind_name) const noexcept -> bool;

  template <IsServiceKind T>
  [[nodiscard]] auto
  As(const Kind::Id kind_id) noexcept -> OptionRef<T&> {
    if (!Is(kind_id)) {
      return None;
    }

    return *static_cast<T*>(this);
  }

  template <IsServiceKind T>
  [[nodiscard]] auto
  As(const Kind::Name& kind_name) noexcept -> OptionRef<T&> {
    if (!Is(kind_name)) {
      return None;
    }

    return *static_cast<T*>(this);
  }

  [[nodiscard]] auto
  SubscribeEvent(const std::string& event) -> Result<Void>;

  [[nodiscard]] auto
  UnsubscribeEvent(const std::string& event) -> Result<Void>;

  auto
  InvokeEvent(const std::string& event, const Dict& data) noexcept -> void;

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
  OnEvent(const std::string& event, const Dict& data) noexcept -> void;

 private:
  Kind kind_;
  Dependencies dependencies_;
  Pin<RunnerContext> runner_context_;
};

using ServicePtr = std::unique_ptr<Service>;
using ServiceFactory =
    std::function<Result<ServicePtr>(const Pin<RunnerContext>)>;

class ServiceMap {
 public:
  using ServiceMapRaw = std::unordered_map<Service::Kind::Id, ServicePtr>;
  using ServiceKindIdMapRaw =
      std::unordered_map<Service::Kind::Name, Service::Kind::Id>;

  explicit ServiceMap() noexcept = default;
  ~ServiceMap() noexcept = default;
  CLASS_KIND_MOVABLE(ServiceMap);

  [[nodiscard]] auto
  AddService(ServicePtr&& service) noexcept -> Result<Void>;

  [[nodiscard]] auto
  GetService(const Service::Kind::Id service_kind_id) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  GetService(const Service::Kind::Name service_kind_name) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  HasService(const Service::Kind::Id service_kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  HasService(const Service::Kind::Name service_kind_name) const noexcept
      -> bool;

  [[nodiscard]] auto
  GetServiceMapRaw() const noexcept -> const ServiceMapRaw&;

  [[nodiscard]] auto
  GetServiceMapRaw() noexcept -> ServiceMapRaw&;

  [[nodiscard]] auto
  GetServiceKindIdMapRaw() const noexcept -> const ServiceKindIdMapRaw&;

  [[nodiscard]] auto
  GetServiceKindIdMapRaw() noexcept -> ServiceKindIdMapRaw&;

 private:
  ServiceMapRaw service_map_;
  ServiceKindIdMapRaw service_kind_id_map_;
};

class ServiceTraverser {
 public:
  using OnVisit = std::function<Result<Void>(Service& service)>;

  /**
   * The "kind_id" is the key and the "kind_name" is the value.
   * If "kind_id" exists, it means visited. Otherwise, it means not visited.
   */
  using VisitMap = std::unordered_map<Service::Kind::Id, Service::Kind::Name>;
  using TraversalStack = std::vector<Service::Kind::Id>;

  explicit ServiceTraverser(
      const ServiceMap::ServiceMapRaw& service_map) noexcept;
  ~ServiceTraverser() noexcept = default;
  CLASS_KIND_PINNABLE(ServiceTraverser);

  [[nodiscard]] auto
  Traverse(const OnVisit& on_visit) noexcept -> Result<Void>;

 private:
  [[nodiscard]] auto
  TraverseRecursive(const Service::Kind& service_kind,
                    const OnVisit& on_visit) noexcept -> Result<Void>;

  const ServiceMap::ServiceMapRaw& service_map_;
  VisitMap visit_map_;
  TraversalStack traversal_stack_;
};

}  // namespace kero

namespace std {

template <>
struct hash<kero::Service::Kind> {
  [[nodiscard]] auto
  operator()(const kero::Service::Kind& kind) const noexcept -> size_t {
    return std::hash<kero::Service::Kind::Id>{}(kind.id);
  }
};

template <>
struct equal_to<kero::Service::Kind> {
  [[nodiscard]] auto
  operator()(const kero::Service::Kind& lhs,
             const kero::Service::Kind& rhs) const noexcept -> bool {
    return lhs.id == rhs.id;
  }
};

}  // namespace std

#endif  // KERO_ENGINE_SERVICE_H
