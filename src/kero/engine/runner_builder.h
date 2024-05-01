#ifndef KERO_ENGINE_RUNNER_BUILDER_H
#define KERO_ENGINE_RUNNER_BUILDER_H

#include "kero/engine/engine.h"
#include "kero/engine/service.h"

namespace kero {

class ThreadRunner;

class RunnerBuilder {
 public:
  explicit RunnerBuilder(EngineContext* engine_context,
                         std::string&& name) noexcept;
  ~RunnerBuilder() noexcept = default;
  CLASS_KIND_MOVABLE(RunnerBuilder);

  [[nodiscard]] auto
  AddService(ServiceFactory&& service_factory) noexcept -> RunnerBuilder&;

  [[nodiscard]] auto
  BuildRunner() const noexcept -> Result<Pin<Runner>>;

  [[nodiscard]] auto
  BuildThreadRunner() const noexcept -> Result<Pin<ThreadRunner>>;

 private:
  std::vector<ServiceFactory> service_factories_;
  std::string name_;
  EngineContext* engine_context_;
};

}  // namespace kero

#endif  // KERO_ENGINE_RUNNER_BUILDER_H
