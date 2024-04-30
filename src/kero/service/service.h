#ifndef KERO_SERVICE_SERVICE_H
#define KERO_SERVICE_SERVICE_H

#include <string>

#include "kero/core/common.h"
#include "kero/core/dict.h"
#include "kero/core/result.h"

namespace kero {

class Agent;

class Service {
 public:
  using Kind = int32_t;
  using Dependencies = std::vector<Kind>;
  using Event = std::string;
  using Subscriptions = std::vector<Event>;

  explicit Service(const Kind kind,
                   Dependencies&& dependencies = {},
                   Subscriptions&& subscriptions = {}) noexcept;
  virtual ~Service() noexcept;
  CLASS_KIND_MOVABLE(Service);

  [[nodiscard]] auto
  GetKind() const noexcept -> Kind;

  [[nodiscard]] auto
  GetDependencies() const noexcept -> const Dependencies&;

  [[nodiscard]] auto
  GetSubscriptions() const noexcept -> const Subscriptions&;

  template <typename T>
    requires std::is_base_of_v<Service, T>
  [[nodiscard]] auto
  Is(const Kind kind) const noexcept -> bool {
    return kind == kind_;
  }

  template <typename T>
    requires std::is_base_of_v<Service, T>
  [[nodiscard]] auto
  As(const Kind kind) noexcept -> OptionRef<T&> {
    if (!Is<T>(kind)) {
      return None;
    }

    return *static_cast<T*>(this);
  }

  /**
   * Default implementation of the `OnCreate` method is empty.
   */
  [[nodiscard]] virtual auto
  OnCreate(Agent& agent) noexcept -> Result<Void>;

  /**
   * Default implementation of the `OnDestroy` method is empty.
   */
  virtual auto
  OnDestroy(Agent& agent) noexcept -> void;

  /**
   * Default implementation of the `OnUpdate` method is empty.
   */
  virtual auto
  OnUpdate(Agent& agent) noexcept -> void;

  /**
   * Default implementation of the `OnEvent` method is empty.
   */
  virtual auto
  OnEvent(Agent& agent, const Event& event, const Dict& data) noexcept -> void;

 private:
  Subscriptions subscriptions_;
  Dependencies dependencies_;
  Kind kind_;
};

using ServicePtr = std::unique_ptr<Service>;

class ServiceFactory {
 public:
  explicit ServiceFactory() noexcept = default;
  virtual ~ServiceFactory() noexcept = default;
  CLASS_KIND_MOVABLE(ServiceFactory);

  [[nodiscard]] virtual auto
  Create() noexcept -> Result<ServicePtr> = 0;
};

}  // namespace kero

#endif  // KERO_SERVICE_SERVICE_H
