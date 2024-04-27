#ifndef SERVER_ENGINE_CONFIG_H
#define SERVER_ENGINE_CONFIG_H

#include <cstdint>

#include "core/core.h"

#include "common.h"

namespace engine {

struct Config final {
  uint16_t port{kUndefinedPort};
  std::string primary_event_loop_name;

  static constexpr uint16_t kUndefinedPort{0};

  explicit Config() noexcept = default;
  ~Config() noexcept = default;
  CLASS_KIND_MOVABLE(Config);

  [[nodiscard]] auto Validate() const noexcept -> Result<Void>;
};

} // namespace engine

#endif // SERVER_ENGINE_CONFIG_H
