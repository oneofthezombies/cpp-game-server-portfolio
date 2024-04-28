#include "utils_linux.h"

#include <cstring>
#include <ostream>

using namespace core;

core::LinuxError::LinuxError(const int code,
                             const std::string_view description) noexcept
    : code{code}, description{description} {}

auto
core::LinuxError::FromErrno() noexcept -> LinuxError {
  const auto code = errno;
  return LinuxError{code, std::string_view{strerror(code)}};
}

auto
core::operator<<(std::ostream &os,
                 const LinuxError &error) noexcept -> std::ostream & {
  os << "LinuxError{";
  os << "code=" << error.code;
  os << ", ";
  os << "description=" << error.description;
  os << "}";
  return os;
}
