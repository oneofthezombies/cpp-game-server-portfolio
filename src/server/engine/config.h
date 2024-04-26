#ifndef SERVER_ENGINE_CONFIG_H
#define SERVER_ENGINE_CONFIG_H

#include <cstdint>
#include <memory>
#include <vector>

#include "core/core.h"

#include "common.h"
#include "session_service.h"

namespace engine {

struct Config final {
  uint16_t port{kUndefinedPort};
  std::unique_ptr<SessionService<>> root_service;
  std::vector<std::unique_ptr<SessionService<>>> services;

  static constexpr uint16_t kUndefinedPort{0};

  explicit Config() noexcept = default;
  ~Config() noexcept = default;
  CLASS_KIND_MOVABLE(Config);

  [[nodiscard]] auto Validate() const noexcept -> Result<core::Void>;
};

} // namespace engine

#endif // SERVER_ENGINE_CONFIG_H