#ifndef KERO_LOG_RUNNER_EVENT_H
#define KERO_LOG_RUNNER_EVENT_H

#include <variant>

#include "kero/log/core.h"

namespace kero {

namespace runner_event {

struct Shutdown final {
  ShutdownConfig config;

  explicit Shutdown(ShutdownConfig&& config) noexcept;
  ~Shutdown() noexcept = default;
  KERO_CLASS_KIND_MOVABLE(Shutdown);
};

}  // namespace runner_event

using RunnerEvent = std::variant<runner_event::Shutdown>;

}  // namespace kero

#endif  // KERO_LOG_RUNNER_EVENT_H
