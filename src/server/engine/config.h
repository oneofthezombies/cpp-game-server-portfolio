#ifndef SERVER_ENGINE_CONFIG_H
#define SERVER_ENGINE_CONFIG_H

#include <cstdint>

#include "common.h"
#include "core/core.h"

namespace engine {

struct Config final {
  u16 port{kUndefinedPort};
  std::string primary_event_loop_name;

  static constexpr u16 kUndefinedPort{0};

  explicit Config() noexcept = default;
  ~Config() noexcept = default;
  CLASS_KIND_MOVABLE(Config);

  [[nodiscard]] auto
  Validate() const noexcept -> Result<Void>;
};

}  // namespace engine

#endif  // SERVER_ENGINE_CONFIG_H
