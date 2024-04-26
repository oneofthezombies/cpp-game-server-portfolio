#include "utils_linux.h"

#include <cstring>
#include <ostream>

using namespace core;

core::LinuxError::LinuxError(const int errno_value,
                             const std::string_view errno_description) noexcept
    : errno_value{errno_value}, errno_description{errno_description} {}

auto core::LinuxError::FromErrno() noexcept -> LinuxError {
  const auto errno_value = errno;
  return LinuxError{errno_value, std::string_view{strerror(errno_value)}};
}

auto core::operator<<(std::ostream &os,
                      const LinuxError &error) noexcept -> std::ostream & {
  os << "LinuxError{";
  os << "errno_value=" << error.errno_value;
  os << ", ";
  os << "errno_description=" << error.errno_description;
  os << "}";
  return os;
}
