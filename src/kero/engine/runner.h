#ifndef KERO_ENGINE_RUNNER_H
#define KERO_ENGINE_RUNNER_H

#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "kero/core/common.h"
#include "kero/core/result.h"
#include "kero/engine/service.h"
#include "kero/engine/service_map.h"

namespace kero {

class ThreadRunner;
struct EngineContext;

class Runner {
 public:
  using EventHandlerMapT = std::unordered_map<std::string /* event */,
                                              std::unordered_set<ServiceKind>>;

  enum : Error::Code {
    kInterrupted = 3,
  };

  explicit Runner(std::string&& name) noexcept;
  ~Runner() noexcept = default;
  CLASS_KIND_PINNABLE(Runner);

  [[nodiscard]] auto
  Run() noexcept -> Result<Void>;

  [[nodiscard]] auto
  GetService(const ServiceKind::Id service_kind_id) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  GetService(const ServiceKind::Name service_kind_name) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  HasService(const ServiceKind::Id service_kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  HasService(const ServiceKind::Name service_kind_name) const noexcept -> bool;

  [[nodiscard]] auto
  HasServiceIs(const ServiceKind::Id service_kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  HasServiceIs(const ServiceKind::Name service_kind_name) const noexcept
      -> bool;

  [[nodiscard]] auto
  SubscribeEvent(const std::string& event,
                 const ServiceKind& kind) -> Result<Void>;

  [[nodiscard]] auto
  UnsubscribeEvent(const std::string& event,
                   const ServiceKind& kind) -> Result<Void>;

  [[nodiscard]] auto
  InvokeEvent(const std::string& event,
              const Dict& data) noexcept -> Result<Void>;

 private:
  [[nodiscard]] auto
  ResolveDependencies() noexcept -> Result<Void>;

  [[nodiscard]] auto
  CreateServices() noexcept -> Result<Void>;

  auto
  DestroyServices() noexcept -> void;

  auto
  UpdateServices() noexcept -> void;

  ServiceMap service_map_;
  EventHandlerMapT event_handler_map_;
  std::string name_;

  friend class RunnerBuilder;
};

class ThreadRunner {
 public:
  explicit ThreadRunner(Pin<Runner> runner) noexcept;
  ~ThreadRunner() noexcept = default;
  CLASS_KIND_MOVABLE(ThreadRunner);

  [[nodiscard]] auto
  Start() -> Result<Void>;

  [[nodiscard]] auto
  Stop() -> Result<Void>;

 private:
  static void
  ThreadMain(Pin<Runner> runner) noexcept;

  std::thread thread_;
  Pin<Runner> runner_;
};

}  // namespace kero

#endif  // KERO_ENGINE_RUNNER_H
