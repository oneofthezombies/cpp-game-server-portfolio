#include "dict.h"

#include <ostream>

#include "kero/log/log_builder.h"

using namespace kero;

kero::Dict::Dict(Data&& data) noexcept : data_{std::move(data)} {}

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
