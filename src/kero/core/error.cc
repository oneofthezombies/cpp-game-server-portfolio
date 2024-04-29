#include "error.h"

using namespace kero;

kero::Error::Error(const Code code,
                   Dict &&details,
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
                  Dict &&details,
                  Error &&cause,
                  std::source_location &&location) noexcept -> Error {
  return Error{code,
               std::move(details),
               std::move(location),
               std::make_unique<Error>(std::move(cause))};
}

auto
kero::Error::From(const Code code,
                  Dict &&details,
                  std::source_location &&location) noexcept -> Error {
  return Error{code, std::move(details), std::move(location), nullptr};
}

auto
kero::Error::From(const Code code,
                  Error &&cause,
                  std::source_location &&location) noexcept -> Error {
  return Error{code,
               Dict{},
               std::move(location),
               std::make_unique<Error>(std::move(cause))};
}

auto
kero::Error::From(Dict &&details,
                  Error &&cause,
                  std::source_location &&location) noexcept -> Error {
  return Error{kFailed,
               std::move(details),
               std::move(location),
               std::make_unique<Error>(std::move(cause))};
}

auto
kero::Error::From(Dict &&details, std::source_location &&location) noexcept
    -> Error {
  return Error{kFailed, std::move(details), std::move(location), nullptr};
}

auto
kero::Error::From(const Code code, std::source_location &&location) noexcept
    -> Error {
  return Error{code, Dict{}, std::move(location), nullptr};
}

auto
kero::Error::From(Error &&cause, std::source_location &&location) noexcept
    -> Error {
  return Error{kPropagated,
               Dict{},
               std::move(location),
               std::make_unique<Error>(std::move(cause))};
}