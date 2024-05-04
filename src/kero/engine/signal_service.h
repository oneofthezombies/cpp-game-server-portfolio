#ifndef KERO_MIDDLEWARE_SIGNAL_SERVICE_H
#define KERO_MIDDLEWARE_SIGNAL_SERVICE_H

#include <atomic>

#include "kero/engine/service.h"

namespace kero {

class SignalService final : public Service {
 public:
  explicit SignalService(const Pin<RunnerContext> runner_context) noexcept;
  virtual ~SignalService() noexcept override = default;
  CLASS_KIND_MOVABLE(SignalService);

  virtual auto
  OnCreate() noexcept -> Result<Void> override;

  virtual auto
  OnDestroy() noexcept -> void override;

  virtual auto
  OnUpdate() noexcept -> void override;

  [[nodiscard]] auto
  IsInterrupted() const noexcept -> bool;

  [[nodiscard]] static auto
  GetKindId() noexcept -> ServiceKindId;

  [[nodiscard]] static auto
  GetKindName() noexcept -> ServiceKindName;

 private:
  static auto
  OnSignal(int signal) noexcept -> void;

  static std::atomic<bool> interrupted_;
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_SIGNAL_SERVICE_H
