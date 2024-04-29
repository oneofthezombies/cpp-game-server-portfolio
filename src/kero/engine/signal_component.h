#ifndef KERO_ENGINE_SIGNAL_COMPONENT_H
#define KERO_ENGINE_SIGNAL_COMPONENT_H

#include <atomic>

#include "kero/engine/component.h"

namespace kero {

class SignalComponent final : public Component {
 public:
  enum : Error::Code {
    kActorComponentNotFound = 1,
  };

  explicit SignalComponent() noexcept;
  virtual ~SignalComponent() noexcept override = default;
  CLASS_KIND_MOVABLE(SignalComponent);

  virtual auto
  OnCreate(Agent& agent) noexcept -> Result<Void> override;

  virtual auto
  OnDestroy(Agent& agent) noexcept -> void override;

  virtual auto
  OnUpdate(Agent& agent) noexcept -> void override;

  [[nodiscard]] auto
  IsInterrupted() const noexcept -> bool;

 private:
  static auto
  OnSignal(int signal) noexcept -> void;

  static std::atomic<bool> interrupted_;
};

}  // namespace kero

#endif  // KERO_ENGINE_SIGNAL_COMPONENT_H
