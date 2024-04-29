#ifndef KERO_ENGINE_SIGNAL_SERVICE_H
#define KERO_ENGINE_SIGNAL_SERVICE_H

#include <atomic>

#include "kero/engine/service.h"

namespace kero {

class SignalService final : public Service {
 public:
  explicit SignalService() noexcept;
  virtual ~SignalService() noexcept override = default;
  CLASS_KIND_MOVABLE(SignalService);

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

#endif  // KERO_ENGINE_SIGNAL_SERVICE_H
