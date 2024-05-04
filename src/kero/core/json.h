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
concept IsJsonType = std::disjunction_v<std::is_same<T, bool>,
                                        std::is_same<T, double>,
                                        std::is_same<T, std::string>>;

template <typename T>
concept IsGetValueType = std::disjunction_v<std::is_same<T, bool>,
                                            std::is_same<T, i8>,
                                            std::is_same<T, i16>,
                                            std::is_same<T, i32>,
                                            std::is_same<T, i64>,
                                            std::is_same<T, u8>,
                                            std::is_same<T, u16>,
                                            std::is_same<T, u32>,
                                            std::is_same<T, u64>,
                                            std::is_same<T, float>,
                                            std::is_same<T, double>>;

template <typename T>
concept IsGetReferenceType = std::disjunction_v<std::is_same<T, const char*>,
                                                std::is_same<T, std::string>>;

template <typename T>
concept IsSetValueType = std::disjunction_v<std::is_same<T, bool>,
                                            std::is_same<T, i8>,
                                            std::is_same<T, i16>,
                                            std::is_same<T, i32>,
                                            std::is_same<T, i64>,
                                            std::is_same<T, u8>,
                                            std::is_same<T, u16>,
                                            std::is_same<T, u32>,
                                            std::is_same<T, u64>,
                                            std::is_same<T, float>,
                                            std::is_same<T, double>,
                                            std::is_same<T, const char*>,
                                            std::is_same<T, std::string_view>>;

template <typename T>
concept IsSetConstRefType = std::disjunction_v<std::is_same<T, std::string>>;

template <typename T>
concept IsSetRValueType = std::disjunction_v<std::is_same<T, std::string>>;

class Json final {
 public:
  using ValueStorage = std::variant<bool, double, std::string>;
  using Data = std::unordered_map<std::string, ValueStorage>;

  explicit Json() noexcept = default;
  explicit Json(Data&& data) noexcept;
  ~Json() noexcept = default;
  CLASS_KIND_MOVABLE(Json);

  template <typename T>
    requires IsGetValueType<T>
  [[nodiscard]] auto
  TryGet(const std::string& key) const noexcept -> Option<T>;

  template <typename T>
    requires IsGetReferenceType<T>
  [[nodiscard]] auto
  TryGet(const std::string& key) const noexcept -> OptionRef<const T&>;

  template <typename T>
    requires IsSetValueType<T>
  [[nodiscard]] auto
  Set(std::string&& key, const T value) noexcept -> Json&;

  template <typename T>
    requires IsSetConstRefType<T>
  [[nodiscard]] auto
  Set(std::string&& key, const T& value) noexcept -> Json&;

  template <typename T>
    requires IsSetRValueType<T>
  [[nodiscard]] auto
  Set(std::string&& key, T&& value) noexcept -> Json&;

  template <typename T>
    requires IsSetValueType<T>
  [[nodiscard]] auto
  TrySet(std::string&& key, const T value) noexcept -> bool;

  template <typename T>
    requires IsSetConstRefType<T>
  [[nodiscard]] auto
  TrySet(std::string&& key, const T& value) noexcept -> bool;

  template <typename T>
    requires IsSetRValueType<T>
  [[nodiscard]] auto
  TrySet(std::string&& key, T&& value) noexcept -> bool;

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
    requires IsJsonType<T>
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
    requires IsJsonType<T>
  [[nodiscard]] auto
  SetImpl(std::string&& key, T&& value) noexcept -> Json& {
    const auto [it, inserted] =
        data_.try_emplace(std::move(key), std::move(value));
    if (!inserted) {
      LogError("Overwriting existing data key: " + key);
      it->second = std::move(value);
    }

    return *this;
  }

  template <typename T>
    requires IsJsonType<T>
  [[nodiscard]] auto
  TrySetImpl(std::string&& key, T&& value) noexcept -> bool {
    const auto [it, inserted] =
        data_.try_emplace(std::move(key), std::move(value));
    return inserted;
  }

  auto
  LogError(std::string&& message,
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

template <>
inline auto
kero::Json::TryGet<bool>(const std::string& key) const noexcept
    -> Option<bool> {
  const auto value = TryGetImpl<bool>(key);
  if (value.IsNone()) {
    return None;
  }

  return Option<bool>::Some(static_cast<bool>(value.Unwrap()));
}

template <>
inline auto
kero::Json::TryGet<i8>(const std::string& key) const noexcept -> Option<i8> {
  const auto value = TryGetImpl<double>(key);
  if (value.IsNone()) {
    return None;
  }

  return Option<i8>::Some(static_cast<i8>(value.Unwrap()));
}

template <>
inline auto
kero::Json::TryGet<i16>(const std::string& key) const noexcept -> Option<i16> {
  const auto value = TryGetImpl<double>(key);
  if (value.IsNone()) {
    return None;
  }

  return Option<i16>::Some(static_cast<i16>(value.Unwrap()));
}

template <>
inline auto
kero::Json::TryGet<i32>(const std::string& key) const noexcept -> Option<i32> {
  const auto value = TryGetImpl<double>(key);
  if (value.IsNone()) {
    return None;
  }

  return Option<i32>::Some(static_cast<i32>(value.Unwrap()));
}

template <>
inline auto
kero::Json::TryGet<i64>(const std::string& key) const noexcept -> Option<i64> {
  const auto value = TryGetImpl<double>(key);
  if (value.IsNone()) {
    return None;
  }

  return Option<i64>::Some(static_cast<i64>(value.Unwrap()));
}

template <>
inline auto
kero::Json::TryGet<std::string>(const std::string& key) const noexcept
    -> OptionRef<const std::string&> {
  const auto value = TryGetImpl<std::string>(key);
  if (value.IsNone()) {
    return None;
  }

  return OptionRef<const std::string&>::Some(value.Unwrap());
}

template <>
inline auto
kero::Json::Set<bool>(std::string&& key, const bool value) noexcept -> Json& {
  return SetImpl<bool>(std::move(key), static_cast<bool>(value));
}

template <>
inline auto
kero::Json::Set<i8>(std::string&& key, const i8 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::Set<i16>(std::string&& key, const i16 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::Set<i32>(std::string&& key, const i32 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::Set<i64>(std::string&& key, const i64 value) noexcept -> Json& {
  if (!IsSafeInteger(value)) {
    LogError("i64 value is too large for JSON. key: " + key +
             ", value: " + std::to_string(value));
    return *this;
  }

  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::Set<u8>(std::string&& key, const u8 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::Set<u16>(std::string&& key, const u16 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::Set<u32>(std::string&& key, const u32 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::Set<u64>(std::string&& key, const u64 value) noexcept -> Json& {
  if (!IsSafeInteger(value)) {
    LogError("u64 value is too large for JSON. key: " + key +
             ", value: " + std::to_string(value));
    return *this;
  }

  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::Set<const char*>(std::string&& key, const char* value) noexcept
    -> Json& {
  if (value == nullptr) {
    LogError("Cannot set null char* value in JSON. key: " + key);
    return *this;
  }

  return SetImpl<std::string>(std::move(key), std::string{value});
}

template <>
inline auto
kero::Json::Set<std::string_view>(std::string&& key,
                                  const std::string_view value) noexcept
    -> Json& {
  return SetImpl<std::string>(std::move(key), std::string{value});
}

template <>
inline auto
kero::Json::Set<std::string>(std::string&& key,
                             const std::string& value) noexcept -> Json& {
  return SetImpl<std::string>(std::move(key), std::string{value});
}

template <>
inline auto
kero::Json::Set<std::string>(std::string&& key, std::string&& value) noexcept
    -> Json& {
  return SetImpl<std::string>(std::move(key), std::move(value));
}

template <>
inline auto
kero::Json::TrySet<bool>(std::string&& key, const bool value) noexcept -> bool {
  return TrySetImpl<bool>(std::move(key), static_cast<bool>(value));
}

template <>
inline auto
kero::Json::TrySet<i8>(std::string&& key, const i8 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::TrySet<i16>(std::string&& key, const i16 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::TrySet<i32>(std::string&& key, const i32 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::TrySet<i64>(std::string&& key, const i64 value) noexcept -> bool {
  if (!IsSafeInteger(value)) {
    return false;
  }

  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::TrySet<u8>(std::string&& key, const u8 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::TrySet<u16>(std::string&& key, const u16 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::TrySet<u32>(std::string&& key, const u32 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::TrySet<u64>(std::string&& key, const u64 value) noexcept -> bool {
  if (!IsSafeInteger(value)) {
    return false;
  }

  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
inline auto
kero::Json::TrySet<const char*>(std::string&& key, const char* value) noexcept
    -> bool {
  if (value == nullptr) {
    return false;
  }

  return TrySetImpl<std::string>(std::move(key), std::string{value});
}

template <>
inline auto
kero::Json::TrySet<std::string_view>(std::string&& key,
                                     const std::string_view value) noexcept
    -> bool {
  return TrySetImpl<std::string>(std::move(key), std::string{value});
}

template <>
inline auto
kero::Json::TrySet<std::string>(std::string&& key,
                                const std::string& value) noexcept -> bool {
  return TrySetImpl<std::string>(std::move(key), std::string{value});
}

template <>
inline auto
kero::Json::TrySet<std::string>(std::string&& key, std::string&& value) noexcept
    -> bool {
  return TrySetImpl<std::string>(std::move(key), std::move(value));
}

auto
operator<<(std::ostream& os, const Json& json) -> std::ostream&;

}  // namespace kero

#endif  // KERO_CORE_JSON_H
