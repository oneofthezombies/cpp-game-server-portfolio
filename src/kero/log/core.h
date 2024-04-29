#ifndef KERO_LOG_CORE_H
#define KERO_LOG_CORE_H

#include <chrono>
#include <source_location>
#include <string>
#include <unordered_map>

#include "kero/core/common.h"

namespace kero {

enum class Level : int8_t {
  kError = 0,
  kWarn = 10,
  kInfo = 20,
  kDebug = 30,
};

[[nodiscard]] auto
LevelToString(const Level level) noexcept -> std::string;

struct Log final {
  std::unordered_map<std::string, std::string> data;
  std::string message;
  std::source_location location;
  Level level;

  explicit Log(std::string&& message,
               std::source_location&& location,
               const Level level) noexcept;
  ~Log() noexcept = default;
  CLASS_KIND_MOVABLE(Log);
};

struct ShutdownConfig final {
  std::chrono::milliseconds timeout;

  ShutdownConfig() noexcept;
  ShutdownConfig(std::chrono::milliseconds&& timeout) noexcept;
  ~ShutdownConfig() noexcept = default;
  CLASS_KIND_MOVABLE(ShutdownConfig);
};

}  // namespace kero

#endif  // KERO_LOG_CORE_H
