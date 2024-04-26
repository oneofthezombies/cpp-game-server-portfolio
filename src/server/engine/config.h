#ifndef SERVER_ENGINE_CONFIG_H
#define SERVER_ENGINE_CONFIG_H

#include <cstdint>
#include <vector>

#include "core/core.h"

#include "common.h"
#include "session_service.h"

namespace engine {

struct Config final {
  uint16_t port{kUndefinedPort};
  SessionServicePtr primary_session_service;
  std::vector<SessionServicePtr> session_services;

  static constexpr uint16_t kUndefinedPort{0};

  explicit Config() noexcept = default;
  ~Config() noexcept = default;
  CLASS_KIND_MOVABLE(Config);

  [[nodiscard]] auto Validate() const noexcept -> Result<Void>;
};

} // namespace engine

#endif // SERVER_ENGINE_CONFIG_H
