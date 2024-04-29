#ifndef KERO_CORE_OPTION_H
#define KERO_CORE_OPTION_H

#include <functional>
#include <utility>
#include <variant>

#include "kero/core/common.h"

namespace kero {

struct OptionNone final {};

constexpr OptionNone None{};

template <typename T>
class OptionRef final {
 public:
  OptionRef(OptionNone) noexcept : data_{std::monostate{}} {}

  OptionRef(T& data) noexcept
    requires std::is_lvalue_reference_v<T>
      : data_{Ref(data)} {}

  ~OptionRef() noexcept = default;
  CLASS_KIND_PINNABLE(OptionRef);

  explicit
  operator bool() const noexcept {
    return IsSome();
  }

  [[nodiscard]] auto
  IsSome() const noexcept -> bool {
    return std::holds_alternative<Ref>(data_);
  }

  [[nodiscard]] auto
  IsNone() const noexcept -> bool {
    return !IsSome();
  }

  /**
   * Unwraps the option, if it is `None` this will cause a crash.
   */
  [[nodiscard]] auto
  Unwrap() const noexcept -> const T& {
    return std::get<Ref>(data_).get();
  }

  /**
   * Unwraps the option, if it is `None` this will cause a crash.
   */
  [[nodiscard]] auto
  Unwrap() noexcept -> T& {
    return std::get<Ref>(data_).get();
  }

 private:
  using Ref = std::reference_wrapper<std::remove_reference_t<T>>;

  std::variant<std::monostate, Ref> data_;
};

template <typename T>
class Option final {
 public:
  Option(OptionNone) noexcept : data_{std::monostate{}} {}

  Option(T&& data) noexcept : data_{std::move(data)} {}

  ~Option() noexcept = default;
  CLASS_KIND_PINNABLE(Option);

  explicit
  operator bool() const noexcept {
    return IsSome();
  }

  [[nodiscard]] auto
  IsSome() const noexcept -> bool {
    return std::holds_alternative<T>(data_);
  }

  [[nodiscard]] auto
  IsNone() const noexcept -> bool {
    return !IsSome();
  }

  /**
   * Unwraps the option, if it is `None` this will cause a crash.
   */
  [[nodiscard]] auto
  TakeUnwrap() noexcept -> T {
    return std::move(std::get<T>(data_));
  }

 private:
  std::variant<std::monostate, T> data_;
};

}  // namespace kero

#endif  // KERO_CORE_OPTION_H
