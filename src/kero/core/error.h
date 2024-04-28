#ifndef KERO_CORE_ERROR_H
#define KERO_CORE_ERROR_H

#include <cstdint>
#include <memory>
#include <source_location>

#include "kero/core/dict.h"

namespace kero {

struct Error final {
  using Code = int32_t;
  using Cause = std::unique_ptr<Error>;

  enum : int32_t {
    kFailed = 1,
    kPropagated = 2,
  };

  Code code;
  Dict details;
  std::source_location location;
  Cause cause;

  explicit Error(const Code code,
                 Dict &&details,
                 std::source_location &&location,
                 Cause &&cause) noexcept;

  ~Error() noexcept = default;
  CLASS_KIND_MOVABLE(Error);

  [[nodiscard]] auto
  Take() noexcept -> Error;

  [[nodiscard]] static auto
  From(const Code code,
       Dict &&details = Dict{},
       Cause &&cause = nullptr,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(Dict &&details,
       Cause &&cause = nullptr,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  [[nodiscard]] static auto
  From(Error &&cause,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;
};

}  // namespace kero

#endif  // KERO_CORE_ERROR_H
