#ifndef KERO_ENGINE_SERVICE_H
#define KERO_ENGINE_SERVICE_H

#include <string>

#include "kero/core/common.h"
#include "kero/core/json.h"
#include "kero/core/result.h"
#include "kero/engine/pin.h"
#include "kero/engine/service_kind.h"

namespace kero {

class RunnerContext;

class Service {
 public:
  using Dependencies = std::vector<ServiceKind>;

  explicit Service(const Pin<RunnerContext> runner_context,
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

  /**
   * Check equality of the service kind id
   */
  [[nodiscard]] auto
  Is(const ServiceKind& kind) const noexcept -> bool;

  [[nodiscard]] auto
  Is(const ServiceKind::Id kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  Is(const ServiceKind::Name& kind_name) const noexcept -> bool;

  template <IsServiceKind T>
  [[nodiscard]] auto
  As(const ServiceKind& kind) noexcept -> OptionRef<T&> {
    return As<T>(kind.id);
  }

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
              const Json& data) noexcept -> Result<Void>;

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
  OnEvent(const std::string& event, const Json& data) noexcept -> void;

 private:
  ServiceKind kind_;
  Dependencies dependencies_;
  Pin<RunnerContext> runner_context_;
};

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
