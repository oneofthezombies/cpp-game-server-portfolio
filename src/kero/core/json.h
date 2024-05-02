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

  [[nodiscard]] auto
  GetOrDefault(const std::string& key, const bool default_value) const noexcept
      -> bool;

  [[nodiscard]] auto
  GetOrDefault(const std::string& key, const i8 default_value) const noexcept
      -> i8;

  [[nodiscard]] auto
  GetOrDefault(const std::string& key, const i16 default_value) const noexcept
      -> i16;

  [[nodiscard]] auto
  GetOrDefault(const std::string& key, const i32 default_value) const noexcept
      -> i32;

  [[nodiscard]] auto
  GetOrDefault(const std::string& key, const i64 default_value) const noexcept
      -> i64;

  [[nodiscard]] auto
  GetOrDefault(const std::string& key, const u8 default_value) const noexcept
      -> u8;

  [[nodiscard]] auto
  GetOrDefault(const std::string& key, const u16 default_value) const noexcept
      -> u16;

  [[nodiscard]] auto
  GetOrDefault(const std::string& key, const u32 default_value) const noexcept
      -> u32;

  [[nodiscard]] auto
  GetOrDefault(const std::string& key, const u64 default_value) const noexcept
      -> u64;

  [[nodiscard]] auto
  GetOrDefault(const std::string& key, const float default_value) const noexcept
      -> float;

  [[nodiscard]] auto
  GetOrDefault(const std::string& key,
               const double default_value) const noexcept -> double;

  [[nodiscard]] auto
  GetOrDefault(const std::string& key,
               const std::string& default_value) const noexcept
      -> const std::string&;

  [[nodiscard]] auto
  TryGetAsBool(const std::string& key) const noexcept -> Option<bool>;

  [[nodiscard]] auto
  TryGetAsI8(const std::string& key) const noexcept -> Option<i8>;

  [[nodiscard]] auto
  TryGetAsI16(const std::string& key) const noexcept -> Option<i16>;

  [[nodiscard]] auto
  TryGetAsI32(const std::string& key) const noexcept -> Option<i32>;

  [[nodiscard]] auto
  TryGetAsI64(const std::string& key) const noexcept -> Option<i64>;

  [[nodiscard]] auto
  TryGetAsU8(const std::string& key) const noexcept -> Option<u8>;

  [[nodiscard]] auto
  TryGetAsU16(const std::string& key) const noexcept -> Option<u16>;

  [[nodiscard]] auto
  TryGetAsU32(const std::string& key) const noexcept -> Option<u32>;

  [[nodiscard]] auto
  TryGetAsU64(const std::string& key) const noexcept -> Option<u64>;

  [[nodiscard]] auto
  TryGetAsF32(const std::string& key) const noexcept -> Option<float>;

  [[nodiscard]] auto
  TryGetAsF64(const std::string& key) const noexcept -> Option<double>;

  [[nodiscard]] auto
  TryGetAsString(const std::string& key) const noexcept
      -> OptionRef<const std::string&>;

  [[nodiscard]] auto
  Set(std::string&& key, const bool value) noexcept -> Json&;

  [[nodiscard]] auto
  Set(std::string&& key, const i8 value) noexcept -> Json&;

  [[nodiscard]] auto
  Set(std::string&& key, const i16 value) noexcept -> Json&;

  [[nodiscard]] auto
  Set(std::string&& key, const i32 value) noexcept -> Json&;

  [[nodiscard]] auto
  Set(std::string&& key, const u8 value) noexcept -> Json&;

  [[nodiscard]] auto
  Set(std::string&& key, const u16 value) noexcept -> Json&;

  [[nodiscard]] auto
  Set(std::string&& key, const u32 value) noexcept -> Json&;

  [[nodiscard]] auto
  Set(std::string&& key, const float value) noexcept -> Json&;

  [[nodiscard]] auto
  Set(std::string&& key, const double value) noexcept -> Json&;

  [[nodiscard]] auto
  Set(std::string&& key, const char* value) noexcept -> Json&;

  [[nodiscard]] auto
  Set(std::string&& key, const std::string_view value) noexcept -> Json&;

  [[nodiscard]] auto
  Set(std::string&& key, const std::string& value) noexcept -> Json&;

  [[nodiscard]] auto
  Set(std::string&& key, std::string&& value) noexcept -> Json&;

  [[nodiscard]] auto
  TrySet(std::string&& key, const bool value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const i8 value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const i16 value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const i32 value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const i64 value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const u8 value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const u16 value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const u32 value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const u64 value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const float value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const double value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const char* value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const std::string_view value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const std::string& value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, std::string&& value) noexcept -> bool;

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
  GetOrDefault(const std::string& key, const T& default_value) const noexcept
      -> const T& {
    const auto found = data_.find(key);
    if (found == data_.end()) {
      return default_value;
    }

    const auto& value = found->second;
    if (!std::holds_alternative<T>(value)) {
      return default_value;
    }

    return std::get<T>(value);
  }

  template <typename T>
    requires IsJsonValueType<T>
  [[nodiscard]] auto
  TryGet(const std::string& key) const noexcept -> OptionRef<const T&> {
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
  Set(std::string&& key, T&& value) noexcept -> Json& {
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
  TrySet(std::string&& key, T&& value) noexcept -> bool {
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
