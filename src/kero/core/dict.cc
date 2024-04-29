#include "dict.h"

#include <ostream>

using namespace kero;

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
kero::operator<<(std::ostream& os, const Dict& dict) -> std::ostream& {
  os << "{";
  for (const auto& [key, value] : dict.data_) {
    os << key << ": ";
    std::visit([&os](const auto& value) { os << value; }, value);
    os << ", ";
  }
  os << "}";
  return os;
}
