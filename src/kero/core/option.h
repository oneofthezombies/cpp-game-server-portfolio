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
      : data_{Wrapper(data)} {}

  ~OptionRef() noexcept = default;
  CLASS_KIND_PINNABLE(OptionRef);

  explicit
  operator bool() const noexcept {
    return IsSome();
  }

  [[nodiscard]] auto
  IsSome() const noexcept -> bool {
    return std::holds_alternative<Wrapper>(data_);
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
    return std::get<Wrapper>(data_).get();
  }

  /**
   * Unwraps the option, if it is `None` this will cause a crash.
   */
  [[nodiscard]] auto
  Unwrap() noexcept -> T& {
    return std::get<Wrapper>(data_).get();
  }

  [[nodiscard]] static auto
  Some(T& data) noexcept -> OptionRef {
    return OptionRef{data};
  }

  [[nodiscard]] static auto
  None() noexcept -> OptionRef {
    return OptionRef{None};
  }

 private:
  using Wrapper = std::reference_wrapper<std::remove_reference_t<T>>;

  std::variant<std::monostate, Wrapper> data_;
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

  [[nodiscard]] static auto
  Some(T&& data) noexcept -> Option {
    return Option{std::move(data)};
  }

  [[nodiscard]] static auto
  None() noexcept -> Option {
    return Option{None};
  }

 private:
  std::variant<std::monostate, T> data_;
};

}  // namespace kero

#endif  // KERO_CORE_OPTION_H
