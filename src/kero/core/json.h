#ifndef KERO_CORE_JSON_H
#define KERO_CORE_JSON_H

#include <source_location>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include "kero/core/common.h"
#include "kero/core/option.h"

namespace kero {

template <typename T>
concept IsJsonValueType = std::disjunction_v<std::is_same<T, bool>,
                                             std::is_same<T, double>,
                                             std::is_same<T, std::string>>;

class Json final {
 public:
  using ValueStorage = std::variant<bool, double, std::string>;
  using Data = std::unordered_map<std::string, ValueStorage>;

  explicit Json() noexcept = default;
  explicit Json(Data&& data) noexcept;
  ~Json() noexcept = default;
  CLASS_KIND_MOVABLE(Json);

  template <typename T>
  [[nodiscard]] auto
  TryGet(const std::string& key) const noexcept -> Option<T> {
    static_assert(false, "Unsupported type for TryGet -> Option");
  }

  template <typename T>
  [[nodiscard]] auto
  TryGet(const std::string& key) const noexcept -> OptionRef<const T&> {
    static_assert(false, "Unsupported type for TryGet -> OptionRef");
  }

  template <typename T>
  [[nodiscard]] auto
  Set(std::string&& key, const T value) noexcept -> Json& {
    static_assert(false, "Unsupported type for Set (const T)");
  }

  template <typename T>
  [[nodiscard]] auto
  Set(std::string&& key, const T& value) noexcept -> Json& {
    static_assert(false, "Unsupported type for Set (const T&)");
  }

  template <typename T>
  [[nodiscard]] auto
  Set(std::string&& key, T&& value) noexcept -> Json& {
    static_assert(false, "Unsupported type for Set (T&&)");
  }

  template <typename T>
  [[nodiscard]] auto
  TrySet(std::string&& key, const T value) noexcept -> bool {
    static_assert(false, "Unsupported type for TrySet (const T)");
  }

  template <typename T>
  [[nodiscard]] auto
  TrySet(std::string&& key, const T& value) noexcept -> bool {
    static_assert(false, "Unsupported type for TrySet (const T&)");
  }

  template <typename T>
  [[nodiscard]] auto
  TrySet(std::string&& key, T&& value) noexcept -> bool {
    static_assert(false, "Unsupported type for TrySet (T&&)");
  }

  [[nodiscard]] auto
  Unset(const std::string& key) noexcept -> Json&;

  [[nodiscard]] auto
  TryUnset(const std::string& key) noexcept -> bool;

  [[nodiscard]] auto
  Has(const std::string& key) const noexcept -> bool;

  [[nodiscard]] auto
  Take() noexcept -> Json;

  [[nodiscard]] auto
  Clone() const noexcept -> Json;

  [[nodiscard]] auto
  AsRaw() const noexcept -> const Data&;

  [[nodiscard]] auto
  AsRaw() noexcept -> Data&;

  static constexpr double kMaxSafeInteger = 9007199254740991.0;
  static constexpr double kMinSafeInteger = -9007199254740991.0;

 private:
  template <typename T>
    requires IsJsonValueType<T>
  [[nodiscard]] auto
  TryGetImpl(const std::string& key) const noexcept -> OptionRef<const T&> {
    const auto found = data_.find(key);
    if (found == data_.end()) {
      return None;
    }

    const auto& value = found->second;
    if (!std::holds_alternative<T>(value)) {
      return None;
    }

    return OptionRef<const T&>{std::get<T>(value)};
  }

  template <typename T>
    requires IsJsonValueType<T>
  [[nodiscard]] auto
  SetImpl(std::string&& key, T&& value) noexcept -> Json& {
    const auto [it, inserted] =
        data_.try_emplace(std::move(key), std::move(value));
    if (!inserted) {
      Warn("Overwriting existing data key: " + key);
      it->second = std::move(value);
    }

    return *this;
  }

  template <typename T>
    requires IsJsonValueType<T>
  [[nodiscard]] auto
  TrySetImpl(std::string&& key, T&& value) noexcept -> bool {
    const auto [it, inserted] =
        data_.try_emplace(std::move(key), std::move(value));
    return inserted;
  }

  auto
  Warn(std::string&& message,
       std::source_location&& location =
           std::source_location::current()) const noexcept -> void;

  [[nodiscard]] auto
  IsSafeInteger(i64 value) const noexcept -> bool;

  [[nodiscard]] auto
  IsSafeInteger(u64 value) const noexcept -> bool;

  Data data_;

  friend auto
  operator<<(std::ostream& os, const Json& json) -> std::ostream&;
};

auto
operator<<(std::ostream& os, const Json& json) -> std::ostream&;

}  // namespace kero

#endif  // KERO_CORE_JSON_H
