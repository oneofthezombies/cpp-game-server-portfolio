#include "dict.h"

#include <ostream>

#include "kero/log/log_builder.h"

using namespace kero;

kero::Dict::Dict(Data&& data) noexcept : data_{std::move(data)} {}

auto
kero::Dict::GetOrDefault(const std::string& key,
                         const bool default_value) const noexcept -> bool {
  return GetOrDefault<bool>(key, default_value);
}

auto
kero::Dict::GetOrDefault(const std::string& key,
                         const double default_value) const noexcept -> double {
  return GetOrDefault<double>(key, default_value);
}

auto
kero::Dict::GetOrDefault(const std::string& key,
                         const std::string& default_value) const noexcept
    -> const std::string& {
  return GetOrDefault<std::string>(key, default_value);
}

auto
kero::Dict::TryGetAsBool(const std::string& key) const noexcept
    -> OptionRef<const bool&> {
  return TryGet<bool>(key);
}

auto
kero::Dict::TryGetAsDouble(const std::string& key) const noexcept
    -> OptionRef<const double&> {
  return TryGet<double>(key);
}

auto
kero::Dict::TryGetAsString(const std::string& key) const noexcept
    -> OptionRef<const std::string&> {
  return TryGet<std::string>(key);
}

auto
kero::Dict::TakeOrDefault(const std::string& key, bool default_value) noexcept
    -> bool {
  return TakeOrDefault<bool>(key, std::move(default_value));
}

auto
kero::Dict::TakeOrDefault(const std::string& key, double default_value) noexcept
    -> double {
  return TakeOrDefault<double>(key, std::move(default_value));
}

auto
kero::Dict::TakeOrDefault(const std::string& key,
                          std::string&& default_value) noexcept -> std::string {
  return TakeOrDefault<std::string>(key, std::move(default_value));
}

auto
kero::Dict::TakeOrDefault(const std::string& key,
                          std::string_view default_value) noexcept
    -> std::string {
  return TakeOrDefault<std::string>(key, std::string{default_value});
}

auto
kero::Dict::TryTakeAsBool(const std::string& key) noexcept -> Option<bool> {
  return TryTake<bool>(key);
}

auto
kero::Dict::TryTakeAsDouble(const std::string& key) noexcept -> Option<double> {
  return TryTake<double>(key);
}

auto
kero::Dict::TryTakeAsString(const std::string& key) noexcept
    -> Option<std::string> {
  return TryTake<std::string>(key);
}

auto
kero::Dict::Set(std::string&& key, bool value) noexcept -> Self& {
  return Set<bool>(std::move(key), std::move(value));
}

auto
kero::Dict::Set(std::string&& key, double value) noexcept -> Self& {
  return Set<double>(std::move(key), std::move(value));
}

auto
kero::Dict::Set(std::string&& key, const char* value) noexcept -> Self& {
  return Set<std::string>(std::move(key), std::string{value});
}

auto
kero::Dict::Set(std::string&& key, const std::string_view value) noexcept
    -> Self& {
  return Set<std::string>(std::move(key), std::string{value});
}

auto
kero::Dict::Set(std::string&& key, std::string&& value) noexcept -> Self& {
  return Set<std::string>(std::move(key), std::move(value));
}

auto
kero::Dict::TrySet(std::string&& key, bool value) noexcept -> bool {
  return TrySet<bool>(std::move(key), std::move(value));
}

auto
kero::Dict::TrySet(std::string&& key, double value) noexcept -> bool {
  return TrySet<double>(std::move(key), std::move(value));
}

auto
kero::Dict::TrySet(std::string&& key, const char* value) noexcept -> bool {
  return TrySet<std::string>(std::move(key), std::string{value});
}

auto
kero::Dict::TrySet(std::string&& key, const std::string_view value) noexcept
    -> bool {
  return TrySet<std::string>(std::move(key), std::string{value});
}

auto
kero::Dict::TrySet(std::string&& key, std::string&& value) noexcept -> bool {
  return TrySet<std::string>(std::move(key), std::move(value));
}

auto
kero::Dict::Has(const std::string& key) const noexcept -> bool {
  return data_.find(key) != data_.end();
}

auto
kero::Dict::Take() noexcept -> Self {
  return std::move(*this);
}

auto
kero::Dict::Clone() const noexcept -> Self {
  Self clone;
  for (const auto& [key, value] : data_) {
    clone.data_.try_emplace(key, value);
  }

  return clone;
}

auto
kero::Dict::AsRaw() const noexcept -> const Data& {
  return data_;
}

auto
kero::Dict::AsRaw() noexcept -> Data& {
  return data_;
}

auto
kero::Dict::Warn(std::string&& message,
                 std::source_location&& location) const noexcept -> void {
  log::Warn(std::move(message), std::move(location)).Log();
}

auto
kero::operator<<(std::ostream& os, const Dict& dict) -> std::ostream& {
  os << "{";
  for (auto it = dict.data_.begin(); it != dict.data_.end(); ++it) {
    os << it->first << ": ";
    std::visit([&os](const auto& value) { os << value; }, it->second);

    if (std::next(it) != dict.data_.end()) {
      os << ", ";
    }
  }
  os << "}";
  return os;
}
