#ifndef KERO_ENGINE_RUNNER_H
#define KERO_ENGINE_RUNNER_H

#include <string>
#include <thread>

#include "kero/core/common.h"
#include "kero/core/result.h"
#include "kero/engine/runner_context.h"
#include "kero/engine/service.h"

namespace kero {

class Runner {
 public:
  explicit Runner(const Pinned<RunnerContext> runner_context) noexcept;
  ~Runner() noexcept = default;
  CLASS_KIND_PINNABLE(Runner);

  [[nodiscard]] auto
  Run() noexcept -> Result<Void>;

  [[nodiscard]] auto
  GetService(const ServiceKind& service_kind) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  GetService(const ServiceKind::Id service_kind_id) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  GetService(const ServiceKind::Name& service_kind_name) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  HasService(const ServiceKind& service_kind) const noexcept -> bool;

  [[nodiscard]] auto
  HasService(const ServiceKind::Id service_kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  HasService(const ServiceKind::Name& service_kind_name) const noexcept -> bool;

  [[nodiscard]] auto
  HasServiceIs(const ServiceKind::Id service_kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  HasServiceIs(const ServiceKind::Name service_kind_name) const noexcept
      -> bool;

  [[nodiscard]] auto
  SubscribeEvent(const std::string& event, const ServiceKind& kind)
      -> Result<Void>;

  [[nodiscard]] auto
  UnsubscribeEvent(const std::string& event, const ServiceKind& kind)
      -> Result<Void>;

  [[nodiscard]] auto
  InvokeEvent(const std::string& event, const Json& data) noexcept
      -> Result<Void>;

 private:
  [[nodiscard]] auto
  ResolveDependencies() noexcept -> Result<Void>;

  [[nodiscard]] auto
  CreateServices() noexcept -> Result<Void>;

  auto
  DestroyServices() noexcept -> void;

  auto
  UpdateServices() noexcept -> void;

  Pinned<RunnerContext> runner_context_;
};

class ThreadRunner {
 public:
  explicit ThreadRunner(Pinned<Runner> runner) noexcept;
  ~ThreadRunner() noexcept = default;
  CLASS_KIND_MOVABLE(ThreadRunner);

  [[nodiscard]] auto
  Start() -> Result<Void>;

  [[nodiscard]] auto
  Stop() -> Result<Void>;

 private:
  static void
  ThreadMain(Pinned<Runner> runner) noexcept;

  std::thread thread_;
  Pinned<Runner> runner_;
};

}  // namespace kero

#endif  // KERO_ENGINE_RUNNER_H
