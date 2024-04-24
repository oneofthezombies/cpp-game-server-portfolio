#include "utils_linux.h"

#include <cstring>
#include <ostream>

using namespace core;

LinuxError::LinuxError(const int code, const std::string_view message) noexcept
    : code{code}, message{message} {}

auto core::LinuxError::FromErrno() noexcept -> LinuxError {
  const auto errno_value = errno;
  return LinuxError{errno_value, std::string_view{strerror(errno_value)}};
}

auto core::operator<<(std::ostream &os, const LinuxError &error) noexcept
    -> std::ostream & {
  os << "LinuxError{";
  os << "code=" << error.code;
  os << ", ";
  os << "message=" << error.message;
  os << "}";
  return os;
}
