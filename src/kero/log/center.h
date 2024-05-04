#ifndef KERO_LOG_CENTER_H
#define KERO_LOG_CENTER_H

#include <iostream>
#include <memory>

#include "transport.h"

namespace kero {

class Center final {
 public:
  explicit Center() noexcept = default;
  ~Center() noexcept = default;
  CLASS_KIND_PINNABLE(Center);

  auto
  UseStreamForLoggingSystemError(std::ostream& stream = std::cerr) noexcept
      -> void;

  auto
  Shutdown(ShutdownConfig&& config = ShutdownConfig{}) noexcept -> void;

  auto
  AddTransport(Own<Transport>&& transport) noexcept -> void;
};

}  // namespace kero

#endif  // KERO_LOG_CENTER_H
