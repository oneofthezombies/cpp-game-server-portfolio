#ifndef KERO_ENGINE_SERVICE_H
#define KERO_ENGINE_SERVICE_H

#include <string>

#include "kero/core/common.h"
#include "kero/core/dict.h"
#include "kero/core/result.h"

namespace kero {

class Agent;

class Service {
 public:
  using Kind = int32_t;

  explicit Service(const Kind kind) noexcept;
  virtual ~Service() noexcept = default;
  CLASS_KIND_MOVABLE(Service);

  [[nodiscard]] auto
  GetKind() const noexcept -> Kind;

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
  OnEvent(Agent& agent, const std::string& event, const Dict& data) noexcept
      -> void;

 private:
  Kind kind_;
};

using ServicePtr = std::unique_ptr<Service>;

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_H
