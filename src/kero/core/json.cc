#include "json.h"

#include <ostream>

#include "kero/log/log_builder.h"

using namespace kero;

kero::Json::Json(Data&& data) noexcept : data_{std::move(data)} {}

auto
kero::Json::GetOrDefault(const std::string& key,
                         const bool default_value) const noexcept -> bool {
  return GetOrDefault<bool>(key, default_value);
}

auto
kero::Json::GetOrDefault(const std::string& key,
                         const i8 default_value) const noexcept -> i8 {
  return static_cast<i8>(
      GetOrDefault<double>(key, static_cast<double>(default_value)));
}

auto
kero::Json::GetOrDefault(const std::string& key,
                         const i16 default_value) const noexcept -> i16 {
  return static_cast<i16>(
      GetOrDefault<double>(key, static_cast<double>(default_value)));
}

auto
kero::Json::GetOrDefault(const std::string& key,
                         const i32 default_value) const noexcept -> i32 {
  return static_cast<i32>(
      GetOrDefault<double>(key, static_cast<double>(default_value)));
}

auto
kero::Json::GetOrDefault(const std::string& key,
                         const i64 default_value) const noexcept -> i64 {
  const auto value = TryGet<double>(key);
  if (value.IsNone()) {
    return default_value;
  }

  return static_cast<i64>(value.Unwrap());
}

auto
kero::Json::GetOrDefault(const std::string& key,
                         const u8 default_value) const noexcept -> u8 {
  return static_cast<u8>(
      GetOrDefault<double>(key, static_cast<double>(default_value)));
}

auto
kero::Json::GetOrDefault(const std::string& key,
                         const u16 default_value) const noexcept -> u16 {
  return static_cast<u16>(
      GetOrDefault<double>(key, static_cast<double>(default_value)));
}

auto
kero::Json::GetOrDefault(const std::string& key,
                         const u32 default_value) const noexcept -> u32 {
  return static_cast<u32>(
      GetOrDefault<double>(key, static_cast<double>(default_value)));
}

auto
kero::Json::GetOrDefault(const std::string& key,
                         const u64 default_value) const noexcept -> u64 {
  const auto value = TryGet<double>(key);
  if (value.IsNone()) {
    return default_value;
  }

  if (value.Unwrap() < 0) {
    log::Warn("u64 value is negative")
        .Data("key", key)
        .Data("value", value.Unwrap())
        .Log();
    return default_value;
  }

  return static_cast<u64>(value.Unwrap());
}

auto
kero::Json::GetOrDefault(const std::string& key,
                         const float default_value) const noexcept -> float {
  return GetOrDefault<double>(key, static_cast<double>(default_value));
}

auto
kero::Json::GetOrDefault(const std::string& key,
                         const double default_value) const noexcept -> double {
  return GetOrDefault<double>(key, default_value);
}

auto
kero::Json::GetOrDefault(const std::string& key,
                         const std::string& default_value) const noexcept
    -> const std::string& {
  return GetOrDefault<std::string>(key, default_value);
}

auto
kero::Json::TryGetAsBool(const std::string& key) const noexcept
    -> Option<bool> {
  using OptionT = Option<bool>;

  auto value = TryGet<bool>(key);
  if (value.IsNone()) {
    return None;
  }
  auto temp = value.Unwrap();
  return OptionT::Some(std::move(temp));
}

auto
kero::Json::TryGetAsI8(const std::string& key) const noexcept -> Option<i8> {
  using OptionT = Option<i8>;

  auto value = TryGet<double>(key);
  if (value.IsNone()) {
    return None;
  }
  auto temp = static_cast<i8>(value.Unwrap());
  return OptionT::Some(std::move(temp));
}

auto
kero::Json::TryGetAsI16(const std::string& key) const noexcept -> Option<i16> {
  using OptionT = Option<i16>;

  auto value = TryGet<double>(key);
  if (value.IsNone()) {
    return None;
  }
  auto temp = static_cast<i16>(value.Unwrap());
  return OptionT::Some(std::move(temp));
}

auto
kero::Json::TryGetAsI32(const std::string& key) const noexcept -> Option<i32> {
  using OptionT = Option<i32>;

  auto value = TryGet<double>(key);
  if (value.IsNone()) {
    return None;
  }
  auto temp = static_cast<i32>(value.Unwrap());
  return OptionT::Some(std::move(temp));
}

auto
kero::Json::TryGetAsI64(const std::string& key) const noexcept -> Option<i64> {
  using OptionT = Option<i64>;

  auto value = TryGet<double>(key);
  if (value.IsNone()) {
    return None;
  }
  auto temp = static_cast<i64>(value.Unwrap());
  return OptionT::Some(std::move(temp));
}

auto
kero::Json::TryGetAsU8(const std::string& key) const noexcept -> Option<u8> {
  using OptionT = Option<u8>;

  auto value = TryGet<double>(key);
  if (value.IsNone()) {
    return None;
  }
  auto temp = static_cast<u8>(value.Unwrap());
  return OptionT::Some(std::move(temp));
}

auto
kero::Json::TryGetAsU16(const std::string& key) const noexcept -> Option<u16> {
  using OptionT = Option<u16>;

  auto value = TryGet<double>(key);
  if (value.IsNone()) {
    return None;
  }
  auto temp = static_cast<u16>(value.Unwrap());
  return OptionT::Some(std::move(temp));
}

auto
kero::Json::TryGetAsU32(const std::string& key) const noexcept -> Option<u32> {
  using OptionT = Option<u32>;

  auto value = TryGet<double>(key);
  if (value.IsNone()) {
    return None;
  }
  auto temp = static_cast<u32>(value.Unwrap());
  return OptionT::Some(std::move(temp));
}

auto
kero::Json::TryGetAsU64(const std::string& key) const noexcept -> Option<u64> {
  using OptionT = Option<u64>;

  auto value = TryGet<double>(key);
  if (value.IsNone()) {
    return None;
  }
  auto temp = static_cast<u64>(value.Unwrap());
  return OptionT::Some(std::move(temp));
}

auto
kero::Json::TryGetAsF32(const std::string& key) const noexcept
    -> Option<float> {
  using OptionT = Option<float>;

  auto value = TryGet<double>(key);
  if (value.IsNone()) {
    return None;
  }
  auto temp = static_cast<float>(value.Unwrap());
  return OptionT::Some(std::move(temp));
}

auto
kero::Json::TryGetAsF64(const std::string& key) const noexcept
    -> Option<double> {
  using OptionT = Option<double>;

  auto value = TryGet<double>(key);
  if (value.IsNone()) {
    return None;
  }
  auto temp = value.Unwrap();
  return OptionT::Some(std::move(temp));
}

auto
kero::Json::TryGetAsString(const std::string& key) const noexcept
    -> OptionRef<const std::string&> {
  return TryGet<std::string>(key);
}

auto
kero::Json::Set(std::string&& key, const bool value) noexcept -> Json& {
  return Set<bool>(std::move(key), static_cast<bool>(value));
}

auto
kero::Json::Set(std::string&& key, const i8 value) noexcept -> Json& {
  return Set<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::Set(std::string&& key, const i16 value) noexcept -> Json& {
  return Set<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::Set(std::string&& key, const i32 value) noexcept -> Json& {
  return Set<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::Set(std::string&& key, const u8 value) noexcept -> Json& {
  return Set<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::Set(std::string&& key, const u16 value) noexcept -> Json& {
  return Set<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::Set(std::string&& key, const u32 value) noexcept -> Json& {
  return Set<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::Set(std::string&& key, const float value) noexcept -> Json& {
  return Set<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::Set(std::string&& key, const double value) noexcept -> Json& {
  return Set<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::Set(std::string&& key, const char* value) noexcept -> Json& {
  return Set<std::string>(std::move(key), std::string{value});
}

auto
kero::Json::Set(std::string&& key, const std::string_view value) noexcept
    -> Json& {
  return Set<std::string>(std::move(key), std::string{value});
}

auto
kero::Json::Set(std::string&& key, const std::string& value) noexcept -> Json& {
  return Set<std::string>(std::move(key), std::string{value});
}

auto
kero::Json::Set(std::string&& key, std::string&& value) noexcept -> Json& {
  return Set<std::string>(std::move(key), std::move(value));
}

auto
kero::Json::TrySet(std::string&& key, const bool value) noexcept -> bool {
  return TrySet<bool>(std::move(key), static_cast<bool>(value));
}

auto
kero::Json::TrySet(std::string&& key, const i8 value) noexcept -> bool {
  return TrySet<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::TrySet(std::string&& key, const i16 value) noexcept -> bool {
  return TrySet<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::TrySet(std::string&& key, const i32 value) noexcept -> bool {
  return TrySet<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::TrySet(std::string&& key, const i64 value) noexcept -> bool {
  if (!IsSafeInteger(value)) {
    log::Warn("i64 value is not safe for conversion to double")
        .Data("key", key)
        .Data("value", value)
        .Log();
    return false;
  }

  return TrySet<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::TrySet(std::string&& key, const u8 value) noexcept -> bool {
  return TrySet<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::TrySet(std::string&& key, const u16 value) noexcept -> bool {
  return TrySet<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::TrySet(std::string&& key, const u32 value) noexcept -> bool {
  return TrySet<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::TrySet(std::string&& key, const u64 value) noexcept -> bool {
  if (!IsSafeInteger(value)) {
    log::Warn("u64 value is not safe for conversion to double")
        .Data("key", key)
        .Data("value", value)
        .Log();
    return false;
  }

  return TrySet<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::TrySet(std::string&& key, const float value) noexcept -> bool {
  return TrySet<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::TrySet(std::string&& key, const double value) noexcept -> bool {
  return TrySet<double>(std::move(key), static_cast<double>(value));
}

auto
kero::Json::TrySet(std::string&& key, const char* value) noexcept -> bool {
  return TrySet<std::string>(std::move(key), std::string{value});
}

auto
kero::Json::TrySet(std::string&& key, const std::string_view value) noexcept
    -> bool {
  return TrySet<std::string>(std::move(key), std::string{value});
}

auto
kero::Json::TrySet(std::string&& key, const std::string& value) noexcept
    -> bool {
  return TrySet<std::string>(std::move(key), std::string{value});
}

auto
kero::Json::TrySet(std::string&& key, std::string&& value) noexcept -> bool {
  return TrySet<std::string>(std::move(key), std::move(value));
}

auto
kero::Json::Unset(const std::string& key) noexcept -> Json& {
  data_.erase(key);
  return *this;
}

auto
kero::Json::TryUnset(const std::string& key) noexcept -> bool {
  return data_.erase(key) > 0;
}

auto
kero::Json::Has(const std::string& key) const noexcept -> bool {
  return data_.find(key) != data_.end();
}

auto
kero::Json::Take() noexcept -> Json {
  return std::move(*this);
}

auto
kero::Json::Clone() const noexcept -> Json {
  Json clone;
  for (const auto& [key, value] : data_) {
    clone.data_.try_emplace(key, value);
  }

  return clone;
}

auto
kero::Json::AsRaw() const noexcept -> const Data& {
  return data_;
}

auto
kero::Json::AsRaw() noexcept -> Data& {
  return data_;
}

auto
kero::Json::Warn(std::string&& message,
                 std::source_location&& location) const noexcept -> void {
  log::Warn(std::move(message), std::move(location)).Log();
}

auto
kero::operator<<(std::ostream& os, const Json& json) -> std::ostream& {
  os << "{";
  for (auto it = json.data_.begin(); it != json.data_.end(); ++it) {
    os << it->first << ": ";
    std::visit([&os](const auto& value) { os << value; }, it->second);

    if (std::next(it) != json.data_.end()) {
      os << ", ";
    }
  }
  os << "}";
  return os;
}
