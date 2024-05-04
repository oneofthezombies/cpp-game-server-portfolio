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
                                             std::unordered_set<ServiceKindId>>;

  explicit RunnerContext() noexcept = default;
  ~RunnerContext() noexcept = default;
  CLASS_KIND_PINNABLE(RunnerContext);

  [[nodiscard]] auto
  SubscribeEvent(const std::string& event,
                 const ServiceKindId service_kind_id) noexcept -> Result<Void>;

  [[nodiscard]] auto
  UnsubscribeEvent(const std::string& event,
                   const ServiceKindId service_kind_id) noexcept
      -> Result<Void>;

  [[nodiscard]] auto
  InvokeEvent(const std::string& event,
              const FlatJson& data) noexcept -> Result<Void>;

  [[nodiscard]] auto
  GetName() const noexcept -> const std::string&;

 private:
  ServiceMap service_map_;
  EventHandlerMap event_handler_map_;
  std::string runner_name_;

  friend class Runner;
  friend class RunnerBuilder;
};

}  // namespace kero

#endif  // KERO_ENGINE_RUNNER_CONTEXT_H
