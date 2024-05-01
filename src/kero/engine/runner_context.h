#ifndef KERO_ENGINE_RUNNER_CONTEXT_H
#define KERO_ENGINE_RUNNER_CONTEXT_H

#include "kero/engine/pin_object_system.h"
#include "kero/engine/service_kind.h"

namespace kero {

class Runner;

class RunnerContext {
 public:
  explicit RunnerContext(const Pin<Runner> runner) noexcept;
  ~RunnerContext() noexcept = default;
  CLASS_KIND_PINNABLE(RunnerContext);

  [[nodiscard]] auto
  GetService(const ServiceKind::Id service_kind_id) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  GetService(const ServiceKind::Name& service_kind_name) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  HasService(const ServiceKind::Id service_kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  HasService(const ServiceKind::Name& service_kind_name) const noexcept -> bool;

  [[nodiscard]] auto
  HasServiceIs(const ServiceKind::Id service_kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  HasServiceIs(const ServiceKind::Name& service_kind_name) const noexcept
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
  Pin<Runner> runner_;
};

using RunnerContextPtr = std::unique_ptr<RunnerContext>;

}  // namespace kero

#endif  // KERO_ENGINE_RUNNER_CONTEXT_H
