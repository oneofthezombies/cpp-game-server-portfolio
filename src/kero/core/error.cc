#include "error.h"

using namespace kero;

kero::Error::Error(const Code code,
                   FlatJson &&details,
                   std::source_location &&location,
                   Cause &&cause) noexcept
    : code{code},
      details{std::move(details)},
      location{std::move(location)},
      cause{std::move(cause)} {}

auto
kero::Error::Take() noexcept -> Error {
  return std::move(*this);
}

auto
kero::Error::From(const Code code,
                  FlatJson &&details,
                  Error &&cause,
                  std::source_location &&location) noexcept -> Error {
  return Error{code,
               std::move(details),
               std::move(location),
               std::make_unique<Error>(std::move(cause))};
}

auto
kero::Error::From(const Code code,
                  FlatJson &&details,
                  std::source_location &&location) noexcept -> Error {
  return Error{code, std::move(details), std::move(location), nullptr};
}

auto
kero::Error::From(const Code code,
                  Error &&cause,
                  std::source_location &&location) noexcept -> Error {
  return Error{code,
               FlatJson{},
               std::move(location),
               std::make_unique<Error>(std::move(cause))};
}

auto
kero::Error::From(FlatJson &&details,
                  Error &&cause,
                  std::source_location &&location) noexcept -> Error {
  return Error{kFailed,
               std::move(details),
               std::move(location),
               std::make_unique<Error>(std::move(cause))};
}

auto
kero::Error::From(const Code code,
                  std::source_location &&location) noexcept -> Error {
  return Error{code, FlatJson{}, std::move(location), nullptr};
}

auto
kero::Error::From(FlatJson &&details,
                  std::source_location &&location) noexcept -> Error {
  return Error{kFailed, std::move(details), std::move(location), nullptr};
}

auto
kero::Error::From(Error &&cause,
                  std::source_location &&location) noexcept -> Error {
  return Error{kPropagated,
               FlatJson{},
               std::move(location),
               std::make_unique<Error>(std::move(cause))};
}

auto
kero::operator<<(std::ostream &os, const Error &error) -> std::ostream & {
  os << "Error{";
  os << "code: " << static_cast<i32>(error.code) << ", ";
  os << "details: " << error.details << ", ";
  os << "location: " << error.location.file_name() << ":"
     << error.location.line() << ":" << error.location.column() << " "
     << error.location.function_name();
  if (error.cause) {
    os << ", ";
    os << "cause: " << *error.cause;
  }
  os << "}";
  return os;
}
