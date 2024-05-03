#ifndef KERO_ENGINE_RUNNER_CONTEXT_H
#define KERO_ENGINE_RUNNER_CONTEXT_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "kero/engine/service_map.h"

namespace kero {

class Runner;

class RunnerContext {
 public:
  using EventHandlerMap = std::unordered_map<std::string /* event */,
                                             std::unordered_set<ServiceKind>>;

  explicit RunnerContext() noexcept = default;
  ~RunnerContext() noexcept = default;
  CLASS_KIND_PINNABLE(RunnerContext);

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
  SubscribeEvent(const std::string& event,
                 const ServiceKind& kind) noexcept -> Result<Void>;

  [[nodiscard]] auto
  UnsubscribeEvent(const std::string& event,
                   const ServiceKind& kind) noexcept -> Result<Void>;

  [[nodiscard]] auto
  InvokeEvent(const std::string& event,
              const Json& data) noexcept -> Result<Void>;

 private:
  ServiceMap service_map_;
  EventHandlerMap event_handler_map_;
  std::string runner_name_;

  friend class Runner;
  friend class RunnerBuilder;
};

}  // namespace kero

#endif  // KERO_ENGINE_RUNNER_CONTEXT_H
