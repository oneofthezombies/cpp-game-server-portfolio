#ifndef SERVER_ENGINE_OPTIONS_H
#define SERVER_ENGINE_OPTIONS_H

#include <cstdint>

#include "core/core.h"

#include "common.h"

namespace engine {

struct Options {
  uint16_t port{kUndefinedPort};

  static constexpr uint16_t kUndefinedPort{0};

  explicit Options() noexcept = default;
  ~Options() noexcept = default;
  CLASS_KIND_COPYABLE(Options);

  [[nodiscard]] auto Validate() const noexcept -> Result<Void>;
};

} // namespace engine

#endif // SERVER_ENGINE_OPTIONS_H
