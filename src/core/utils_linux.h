#ifndef CORE_UTILS_LINUX_H
#define CORE_UTILS_LINUX_H

#include "core.h"
#include <string_view>

struct LinuxError final : private Copyable, Movable {
  // `errno`
  int errno_value;

  // `strerror(errno)`
  std::string_view errno_description;

  [[nodiscard]] static auto FromErrno() noexcept -> LinuxError;

private:
  LinuxError(const int code, const std::string_view message) noexcept;
};

auto operator<<(std::ostream &os, const LinuxError &error) noexcept
    -> std::ostream &;

#endif // CORE_UTILS_LINUX_H
