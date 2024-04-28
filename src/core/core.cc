#include "core.h"

#include <ostream>

#include "common.h"

using namespace core;

auto
core::operator<<(std::ostream &os, const Void &) -> std::ostream & {
  os << "Void";
  return os;
}

auto
core::DebugErrorDetails(std::ostream &os, const ErrorDetails &details) -> void {
  os << "Details{";
  for (auto it = details.begin(); it != details.end(); ++it) {
    os << it->first;
    os << "=";
    os << it->second;
    if (std::next(it) != details.end()) {
      os << ", ";
    }
  }
  os << "}";
}

auto
core::operator<<(std::ostream &os, const Error &error) -> std::ostream & {
  os << "Error{";
  os << "code=";
  os << error.code;
  os << ", ";
  os << "details=";
  DebugErrorDetails(os, error.details);
  os << ", ";
  os << "location=";
  os << error.location.file_name();
  os << ":";
  os << error.location.line();
  os << ":";
  os << error.location.column();
  os << ":";
  os << error.location.function_name();
  if (error.cause) {
    os << ", ";
    os << "cause=";
    os << *error.cause;
  }
  os << "}";
  return os;
}

core::Error::Error(const ErrorCode code,
                   ErrorDetails &&details,
                   std::source_location &&location,
                   ErrorCause &&cause) noexcept
    : code{code},
      details{std::move(details)},
      cause{std::move(cause)},
      location{location} {}

auto
core::Error::From(const ErrorCode code,
                  ErrorDetails &&details,
                  std::source_location &&location) noexcept -> Error {
  return Error{
      code,
      std::move(details),
      std::move(location),
      nullptr,
  };
}

auto
core::Error::From(Error &&cause,
                  std::source_location &&location) noexcept -> Error {
  return Error{
      kErrorPropagated,
      {},
      std::move(location),
      std::make_unique<Error>(std::move(cause)),
  };
}
