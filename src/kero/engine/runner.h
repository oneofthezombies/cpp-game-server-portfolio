#ifndef KERO_ENGINE_RUNNER_H
#define KERO_ENGINE_RUNNER_H

#include <thread>

#include "kero/core/common.h"
#include "kero/core/result.h"
#include "kero/engine/runner_context.h"
#include "kero/engine/service.h"

namespace kero {

class Runner {
 public:
  explicit Runner(Own<RunnerContext>&& runner_context) noexcept;
  ~Runner() noexcept = default;
  KERO_CLASS_KIND_PINNABLE(Runner);

  [[nodiscard]] auto
  Run() noexcept -> Result<Void>;

 private:
  Own<RunnerContext> runner_context_;
};

class ThreadRunner {
 public:
  explicit ThreadRunner(Own<Runner>&& runner) noexcept;
  ~ThreadRunner() noexcept = default;
  KERO_CLASS_KIND_MOVABLE(ThreadRunner);

  [[nodiscard]] auto
  Start() -> Result<Void>;

  [[nodiscard]] auto
  Stop() -> Result<Void>;

 private:
  static void
  ThreadMain(const Borrow<Runner> runner) noexcept;

  std::thread thread_;
  Own<Runner> runner_;
};

}  // namespace kero

#endif  // KERO_ENGINE_RUNNER_H
