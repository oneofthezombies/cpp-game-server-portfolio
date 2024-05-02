#ifndef KERO_CORE_UTILS_LINUX_H
#define KERO_CORE_UTILS_LINUX_H

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
  IntoJson() const noexcept -> Json;

  [[nodiscard]] static auto
  FromErrno() noexcept -> Errno;

 private:
  explicit Errno(const Value code, const std::string_view description) noexcept;
};

auto
operator<<(std::ostream& os, const Errno& err) -> std::ostream&;

}  // namespace kero

#endif  // KERO_CORE_UTILS_LINUX_H