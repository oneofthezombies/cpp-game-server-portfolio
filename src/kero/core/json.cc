#include "json.h"

#include <ostream>

#include "kero/log/log_builder.h"

using namespace kero;

kero::Json::Json(Data&& data) noexcept : data_{std::move(data)} {}

template <>
auto
kero::Json::TryGet(const std::string& key) const noexcept -> Option<bool> {
  const auto value = TryGetImpl<bool>(key);
  if (value.IsNone()) {
    return None;
  }

  return Option<bool>::Some(static_cast<bool>(value.Unwrap()));
}

template <>
auto
kero::Json::TryGet(const std::string& key) const noexcept -> Option<i8> {
  const auto value = TryGetImpl<double>(key);
  if (value.IsNone()) {
    return None;
  }

  return Option<i8>::Some(static_cast<i8>(value.Unwrap()));
}

template <>
auto
kero::Json::TryGet(const std::string& key) const noexcept -> Option<i16> {
  const auto value = TryGetImpl<double>(key);
  if (value.IsNone()) {
    return None;
  }

  return Option<i16>::Some(static_cast<i16>(value.Unwrap()));
}

template <>
auto
kero::Json::TryGet(const std::string& key) const noexcept -> Option<i32> {
  const auto value = TryGetImpl<double>(key);
  if (value.IsNone()) {
    return None;
  }

  return Option<i32>::Some(static_cast<i32>(value.Unwrap()));
}

template <>
auto
kero::Json::TryGet(const std::string& key) const noexcept -> Option<i64> {
  const auto value = TryGetImpl<double>(key);
  if (value.IsNone()) {
    return None;
  }

  return Option<i64>::Some(static_cast<i64>(value.Unwrap()));
}

template <>
auto
kero::Json::TryGet(const std::string& key) const noexcept
    -> OptionRef<const std::string&> {
  const auto value = TryGetImpl<std::string>(key);
  if (value.IsNone()) {
    return None;
  }

  return OptionRef<const std::string&>::Some(value.Unwrap());
}

template <>
auto
kero::Json::Set<bool>(std::string&& key, const bool value) noexcept -> Json& {
  return SetImpl<bool>(std::move(key), static_cast<bool>(value));
}

template <>
auto
kero::Json::Set<i8>(std::string&& key, const i8 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set<i16>(std::string&& key, const i16 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set<i32>(std::string&& key, const i32 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set<i64>(std::string&& key, const i64 value) noexcept -> Json& {
  if (!IsSafeInteger(value)) {
    log::Error("i64 value is too large for JSON")
        .Data("key", key)
        .Data("value", value)
        .Log();
    return *this;
  }

  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set<u8>(std::string&& key, const u8 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set<u16>(std::string&& key, const u16 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set<u32>(std::string&& key, const u32 value) noexcept -> Json& {
  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set<u64>(std::string&& key, const u64 value) noexcept -> Json& {
  if (!IsSafeInteger(value)) {
    log::Error("u64 value is too large for JSON")
        .Data("key", key)
        .Data("value", value)
        .Log();
    return *this;
  }

  return SetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::Set<const char*>(std::string&& key, const char* value) noexcept
    -> Json& {
  if (value == nullptr) {
    log::Error("Cannot set null char* value in JSON").Data("key", key).Log();
    return *this;
  }

  return SetImpl<std::string>(std::move(key), std::string{value});
}

template <>
auto
kero::Json::Set<std::string>(std::string&& key,
                             const std::string& value) noexcept -> Json& {
  return SetImpl<std::string>(std::move(key), std::string{value});
}

template <>
auto
kero::Json::Set<std::string>(std::string&& key, std::string&& value) noexcept
    -> Json& {
  return SetImpl<std::string>(std::move(key), std::move(value));
}

template <>
auto
kero::Json::TrySet<bool>(std::string&& key, const bool value) noexcept -> bool {
  return TrySetImpl<bool>(std::move(key), static_cast<bool>(value));
}

template <>
auto
kero::Json::TrySet<i8>(std::string&& key, const i8 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet<i16>(std::string&& key, const i16 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet<i32>(std::string&& key, const i32 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet<i64>(std::string&& key, const i64 value) noexcept -> bool {
  if (!IsSafeInteger(value)) {
    return false;
  }

  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet<u8>(std::string&& key, const u8 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet<u16>(std::string&& key, const u16 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet<u32>(std::string&& key, const u32 value) noexcept -> bool {
  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
}

template <>
auto
kero::Json::TrySet<u64>(std::string&& key, const u64 value) noexcept -> bool {
  if (!IsSafeInteger(value)) {
    return false;
  }

  return TrySetImpl<double>(std::move(key), static_cast<double>(value));
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
