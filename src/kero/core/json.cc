#include "json.h"

#include <ostream>

#include "kero/log/log_builder.h"

using namespace kero;

kero::Json::Json(Data&& data) noexcept : data_{std::move(data)} {}

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
kero::Json::LogError(std::string&& message,
                     std::source_location&& location) const noexcept -> void {
  log::Error(std::move(message), std::move(location)).Log();
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
