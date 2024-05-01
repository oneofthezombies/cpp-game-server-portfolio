#ifndef KERO_ENGINE_SERVICE_H
#define KERO_ENGINE_SERVICE_H

#include <string>

#include "kero/core/common.h"
#include "kero/core/dict.h"
#include "kero/core/result.h"
#include "kero/engine/runner_context.h"
#include "kero/engine/service_kind.h"

namespace kero {

class Service {
 public:
  using Dependencies = std::vector<ServiceKind>;

  explicit Service(RunnerContextPtr&& runner_context,
                   const ServiceKind& kind,
                   Dependencies&& dependencies) noexcept;

  virtual ~Service() noexcept = default;
  CLASS_KIND_MOVABLE(Service);

  [[nodiscard]] auto
  GetKind() const noexcept -> const ServiceKind&;

  [[nodiscard]] auto
  GetDependencies() const noexcept -> const Dependencies&;

  [[nodiscard]] auto
  GetRunnerContext() noexcept -> RunnerContext&;

  [[nodiscard]] auto
  Is(const ServiceKind::Id kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  Is(const ServiceKind::Name& kind_name) const noexcept -> bool;

  template <IsServiceKind T>
  [[nodiscard]] auto
  As(const ServiceKind::Id kind_id) noexcept -> OptionRef<T&> {
    if (!Is(kind_id)) {
      return None;
    }

    return *static_cast<T*>(this);
  }

  template <IsServiceKind T>
  [[nodiscard]] auto
  As(const ServiceKind::Name& kind_name) noexcept -> OptionRef<T&> {
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
  InvokeEvent(const std::string& event,
              const Dict& data) noexcept -> Result<Void>;

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
  ServiceKind kind_;
  Dependencies dependencies_;
  RunnerContextPtr runner_context_;
};

using ServicePtr = std::unique_ptr<Service>;
using ServiceFactory = std::function<Result<ServicePtr>(RunnerContextPtr)>;

}  // namespace kero

namespace std {

template <>
struct hash<kero::ServiceKind> {
  [[nodiscard]] auto
  operator()(const kero::ServiceKind& kind) const noexcept -> size_t {
    return std::hash<kero::ServiceKind::Id>{}(kind.id);
  }
};

template <>
struct equal_to<kero::ServiceKind> {
  [[nodiscard]] auto
  operator()(const kero::ServiceKind& lhs,
             const kero::ServiceKind& rhs) const noexcept -> bool {
    return lhs.id == rhs.id;
  }
};

}  // namespace std

#endif  // KERO_ENGINE_SERVICE_H
