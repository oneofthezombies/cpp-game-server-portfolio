#ifndef KERO_ENGINE_ENGINE_H
#define KERO_ENGINE_ENGINE_H

#include "kero/core/common.h"
#include "kero/engine/engine_context.h"

namespace kero {

class RunnerBuilder;

class Engine {
 public:
  explicit Engine() noexcept = default;
  ~Engine() noexcept = default;
  CLASS_KIND_PINNABLE(Engine);

  [[nodiscard]] auto
  CreateRunnerBuilder(std::string&& runner_name) -> RunnerBuilder;

 private:
  EngineContext engine_context_{};
};

}  // namespace kero

#endif  // KERO_ENGINE_ENGINE_H
