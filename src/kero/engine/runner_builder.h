#ifndef KERO_ENGINE_RUNNER_BUILDER_H
#define KERO_ENGINE_RUNNER_BUILDER_H

#include "kero/core/common.h"
#include "kero/engine/engine.h"
#include "kero/engine/engine_context.h"
#include "kero/engine/runner.h"
#include "kero/engine/service_factory.h"

namespace kero {

class ThreadRunner;

class RunnerBuilder {
 public:
  explicit RunnerBuilder(const Borrow<EngineContext> engine_context,
                         std::string&& runner_name) noexcept;
  ~RunnerBuilder() noexcept = default;
  CLASS_KIND_PINNABLE(RunnerBuilder);

  [[nodiscard]] auto
  AddServiceFactory(Own<ServiceFactory>&& service_factory) noexcept
      -> RunnerBuilder&;

  [[nodiscard]] auto
  BuildRunner() noexcept -> Result<Pin<Runner>>;

  [[nodiscard]] auto
  BuildThreadRunner() noexcept -> Result<Pin<ThreadRunner>>;

 private:
  std::vector<Own<ServiceFactory>> service_factories_;
  std::string runner_name_;
  Borrow<EngineContext> engine_context_;
};

}  // namespace kero

#endif  // KERO_ENGINE_RUNNER_BUILDER_H
