#ifndef KERO_ENGINE_ENGINE_H
#define KERO_ENGINE_ENGINE_H

#include "kero/core/common.h"
#include "kero/engine/engine_context.h"

namespace kero {

class RunnerBuilder;
class Service;

class Engine {
 public:
  explicit Engine() noexcept;
  ~Engine() noexcept = default;
  KERO_CLASS_KIND_PINNABLE(Engine);

  [[nodiscard]] auto
  CreateRunnerBuilder(std::string&& runner_name) -> RunnerBuilder;

  [[nodiscard]] auto
  Start() -> Result<Void>;

  [[nodiscard]] auto
  Stop() -> Result<Void>;

 private:
  Own<EngineContext> engine_context_{};

  friend class ActorServiceFactory;
};

}  // namespace kero

#endif  // KERO_ENGINE_ENGINE_H
