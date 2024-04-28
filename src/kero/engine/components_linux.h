#ifndef KERO_ENGINE_COMPONENTS_LINUX_H
#define KERO_ENGINE_COMPONENTS_LINUX_H

#include "kero/engine/component.h"

namespace kero {

class SignalComponent final : Component {
 public:
  explicit SignalComponent() noexcept;
  virtual ~SignalComponent() noexcept override = default;
  CLASS_KIND_MOVABLE(SignalComponent);

  virtual auto
  OnCreate(Engine& engine) noexcept -> void override;

  virtual auto
  OnDestroy(Engine& engine) noexcept -> void override;

  virtual auto
  OnUpdate(Engine& engine) noexcept -> void override;
};

}  // namespace kero

#endif  // KERO_ENGINE_COMPONENTS_LINUX_H
