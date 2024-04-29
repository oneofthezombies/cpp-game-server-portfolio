#ifndef KERO_ENGINE_COMPONENT_H
#define KERO_ENGINE_COMPONENT_H

#include <string>

#include "kero/core/common.h"
#include "kero/core/dict.h"
#include "kero/core/result.h"

namespace kero {

class Agent;

class Component {
 public:
  using Kind = int32_t;

  explicit Component(const Kind kind) noexcept;
  virtual ~Component() noexcept = default;
  CLASS_KIND_MOVABLE(Component);

  [[nodiscard]] auto
  GetKind() const noexcept -> Kind;

  template <typename T>
    requires std::is_base_of_v<Component, T>
  [[nodiscard]] auto
  Is(const Kind kind) const noexcept -> bool {
    return kind == kind_;
  }

  template <typename T>
    requires std::is_base_of_v<Component, T>
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

using ComponentPtr = std::unique_ptr<Component>;

}  // namespace kero

#endif  // KERO_ENGINE_COMPONENT_H
