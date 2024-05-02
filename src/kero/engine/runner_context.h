#ifndef KERO_ENGINE_RUNNER_CONTEXT_H
#define KERO_ENGINE_RUNNER_CONTEXT_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "kero/engine/service_map.h"

namespace kero {

class Runner;

struct RunnerContext {
  using EventHandlerMap = std::unordered_map<std::string /* event */,
                                             std::unordered_set<ServiceKind>>;

  ServiceMap service_map;
  EventHandlerMap event_handler_map;
  std::string runner_name;

  explicit RunnerContext() noexcept = default;
  ~RunnerContext() noexcept = default;
  CLASS_KIND_PINNABLE(RunnerContext);
};

}  // namespace kero

#endif  // KERO_ENGINE_RUNNER_CONTEXT_H
