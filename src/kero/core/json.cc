#include "json.h"

#include <ostream>

#include "kero/log/log_builder.h"

using namespace kero;

kero::Json::Json(Data&& data) noexcept : data_{std::move(data)} {}

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
kero::Json::SetBool(std::string&& key, const bool value) noexcept -> Json& {
  return SetImpl<bool>(std::move(key), static_cast<bool>(value));
}

template <>
auto
kero::Json::Set(std::string&& key, const bool value) noexcept -> Json& {
  return SetBool(std::move(key), value);
}

auto
kero::Json::SetI8(std::string&& key, const i8 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set(std::string&& key, const i8 value) noexcept -> Json& {
  return SetI8(std::move(key), value);
}

auto
kero::Json::SetI16(std::string&& key, const i16 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set(std::string&& key, const i16 value) noexcept -> Json& {
  return SetI16(std::move(key), value);
}

auto
kero::Json::SetI32(std::string&& key, const i32 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set(std::string&& key, const i32 value) noexcept -> Json& {
  return SetI32(std::move(key), value);
}

auto
kero::Json::SetU8(std::string&& key, const u8 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set(std::string&& key, const u8 value) noexcept -> Json& {
  return SetU8(std::move(key), value);
}

auto
kero::Json::SetU16(std::string&& key, const u16 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set(std::string&& key, const u16 value) noexcept -> Json& {
  return SetU16(std::move(key), value);
}

auto
kero::Json::SetU32(std::string&& key, const u32 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set(std::string&& key, const u32 value) noexcept -> Json& {
  return SetU32(std::move(key), value);
}

auto
kero::Json::SetF32(std::string&& key, const float value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set(std::string&& key, const float value) noexcept -> Json& {
  return SetF32(std::move(key), value);
}

auto
kero::Json::SetF64(std::string&& key, const double value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set(std::string&& key, const double value) noexcept -> Json& {
  return SetF64(std::move(key), value);
}

auto
kero::Json::Set(std::string&& key, const char* value) noexcept -> Json& {
  return SetImpl<std::string>(std::move(key), std::string{value});
}

auto
kero::Json::Set(std::string&& key,
                const std::string_view value) noexcept -> Json& {
  return SetImpl<std::string>(std::move(key), std::string{value});
}

auto
kero::Json::Set(std::string&& key, const std::string& value) noexcept -> Json& {
  return SetImpl<std::string>(std::move(key), std::string{value});
}

auto
kero::Json::Set(std::string&& key, std::string&& value) noexcept -> Json& {
  return SetImpl<std::string>(std::move(key), std::move(value));
}

auto
kero::Json::TrySetBool(std::string&& key, const bool value) noexcept -> bool {
  return TrySetImpl<bool>(std::move(key), static_cast<bool>(value));
}

template <>
auto
kero::Json::TrySet(std::string&& key, const bool value) noexcept -> bool {
  return TrySetBool(std::move(key), value);
}

auto
kero::Json::TrySetI8(std::string&& key, const i8 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet(std::string&& key, const i8 value) noexcept -> bool {
  return TrySetI8(std::move(key), value);
}

auto
kero::Json::TrySetI16(std::string&& key, const i16 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet(std::string&& key, const i16 value) noexcept -> bool {
  return TrySetI16(std::move(key), value);
}

auto
kero::Json::TrySetI32(std::string&& key, const i32 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet(std::string&& key, const i32 value) noexcept -> bool {
  return TrySetI32(std::move(key), value);
}

auto
kero::Json::TrySetI64(std::string&& key, const i64 value) noexcept -> bool {
  if (!IsSafeInteger(value)) {
    log::Warn("i64 value is not safe for conversion to double")
        .Data("key", key)
        .Data("value", value)
        .Log();
    return false;
  }

  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet(std::string&& key, const i64 value) noexcept -> bool {
  return TrySetI64(std::move(key), value);
}

auto
kero::Json::TrySetU8(std::string&& key, const u8 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet(std::string&& key, const u8 value) noexcept -> bool {
  return TrySetU8(std::move(key), value);
}

auto
kero::Json::TrySetU16(std::string&& key, const u16 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet(std::string&& key, const u16 value) noexcept -> bool {
  return TrySetU16(std::move(key), value);
}

auto
kero::Json::TrySetU32(std::string&& key, const u32 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet(std::string&& key, const u32 value) noexcept -> bool {
  return TrySetU32(std::move(key), value);
}

auto
kero::Json::TrySetU64(std::string&& key, const u64 value) noexcept -> bool {
  if (!IsSafeInteger(value)) {
    log::Warn("u64 value is not safe for conversion to double")
        .Data("key", key)
        .Data("value", value)
        .Log();
    return false;
  }

  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet(std::string&& key, const u64 value) noexcept -> bool {
  return TrySetU64(std::move(key), value);
}

auto
kero::Json::TrySetF32(std::string&& key, const float value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet(std::string&& key, const float value) noexcept -> bool {
  return TrySetF32(std::move(key), value);
}

auto
kero::Json::TrySetF64(std::string&& key, const double value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet(std::string&& key, const double value) noexcept -> bool {
  return TrySetF64(std::move(key), value);
}

auto
kero::Json::TrySet(std::string&& key, const char* value) noexcept -> bool {
  return TrySetImpl<std::string>(std::move(key), std::string{value});
}

auto
kero::Json::TrySet(std::string&& key,
                   const std::string_view value) noexcept -> bool {
  return TrySetImpl<std::string>(std::move(key), std::string{value});
}

auto
kero::Json::TrySet(std::string&& key,
                   const std::string& value) noexcept -> bool {
  return TrySetImpl<std::string>(std::move(key), std::string{value});
}

auto
kero::Json::TrySet(std::string&& key, std::string&& value) noexcept -> bool {
  return TrySetImpl<std::string>(std::move(key), std::move(value));
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
