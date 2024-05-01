#ifndef KERO_ENGINE_ENGINE_H
#define KERO_ENGINE_ENGINE_H

#include "actor_system.h"
#include "kero/core/common.h"
#include "kero/core/result.h"
#include "kero/engine/pin_object_system.h"
#include "kero/engine/runner.h"

namespace kero {

class Engine {
 public:
  explicit Engine() noexcept = default;
  ~Engine() noexcept = default;
  CLASS_KIND_PINNABLE(Engine);

  [[nodiscard]] auto
  CreateThreadRunner(std::string&& name) -> Result<Pin<ThreadRunner>>;

  [[nodiscard]] auto
  CreateRunner(std::string&& name) -> Result<Pin<Runner>>;

  [[nodiscard]] static auto
  Global() -> Engine&;

 private:
  PinObjectSystem pin_object_system_;
  ActorSystem actor_system_;
};

}  // namespace kero

#endif  // KERO_ENGINE_ENGINE_H
