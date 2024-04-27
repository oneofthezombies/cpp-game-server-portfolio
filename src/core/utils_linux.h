#ifndef CORE_UTILS_LINUX_H
#define CORE_UTILS_LINUX_H

#include "core.h"
#include <string_view>

namespace core {

struct LinuxError final {
  // `errno`
  int code;

  // `strerror(errno)`
  std::string_view description;

  ~LinuxError() noexcept = default;
  CLASS_KIND_MOVABLE(LinuxError);

  [[nodiscard]] static auto FromErrno() noexcept -> LinuxError;

private:
  LinuxError(const int code, const std::string_view description) noexcept;
};

auto operator<<(std::ostream &os, const LinuxError &error) noexcept
    -> std::ostream &;

} // namespace core

#endif // CORE_UTILS_LINUX_H
