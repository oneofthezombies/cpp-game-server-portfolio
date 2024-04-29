#ifndef KERO_ENGINE_UTILS_LINUX_H
#define KERO_ENGINE_UTILS_LINUX_H

#include <source_location>

#include "kero/core/common.h"
#include "kero/core/result.h"

namespace kero {

struct Fd final {
  using Value = int;

  enum : Error::Code {
    kInvalidValue = 1,
    kGetStatusFailed,
    kSetStatusFailed,
  };

  [[nodiscard]] static auto
  IsValid(const Value fd) noexcept -> bool;

  [[nodiscard]] static auto
  Close(const Value fd) noexcept -> Result<Void>;

  [[nodiscard]] static auto
  UpdateNonBlocking(const Value fd) noexcept -> Result<Void>;

  static constexpr auto kUnspecifiedInitialValue = -1;
};

struct Errno final {
  using Value = int;

  Value code;
  std::string_view description;

  ~Errno() noexcept = default;
  CLASS_KIND_COPYABLE(Errno);

  [[nodiscard]] auto
  IntoDict() const noexcept -> Dict;

  [[nodiscard]] static auto
  FromErrno() noexcept -> Errno;

 private:
  explicit Errno(const Value code, const std::string_view description) noexcept;
};

}  // namespace kero

#endif  // KERO_ENGINE_UTILS_LINUX_H
