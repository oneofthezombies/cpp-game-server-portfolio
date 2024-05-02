#ifndef KERO_CORE_ERROR_H
#define KERO_CORE_ERROR_H

#include <source_location>

#include "kero/core/json.h"

namespace kero {

struct Error final {
  using Code = i32;
  using Cause = Owned<Error>;

  enum : i32 {
    kFailed = 1,
    kPropagated = 2,
  };

  Code code;
  Json details;
  std::source_location location;
  Cause cause;

  explicit Error(const Code code,
                 Json &&details,
                 std::source_location &&location,
                 Cause &&cause) noexcept;

  ~Error() noexcept = default;
  CLASS_KIND_MOVABLE(Error);

  [[nodiscard]] auto
  Take() noexcept -> Error;

  [[nodiscard]] static auto
  From(const Code code,
       Json &&details,
       Error &&cause,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(const Code code,
       Json &&details,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(const Code code,
       Error &&cause,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(Json &&details,
       Error &&cause,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(const Code code,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(Json &&details,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(Error &&cause,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;
};

auto
operator<<(std::ostream &os, const Error &error) -> std::ostream &;

}  // namespace kero

#endif  // KERO_CORE_ERROR_H
