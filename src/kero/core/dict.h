#ifndef KERO_CORE_DICT_H
#define KERO_CORE_DICT_H

#include <cassert>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include "kero/core/common.h"
#include "kero/core/option.h"

namespace kero {

template <typename T>
concept IsDictValueType = std::disjunction_v<std::is_same<T, bool>,
                                             std::is_same<T, int8_t>,
                                             std::is_same<T, int16_t>,
                                             std::is_same<T, int32_t>,
                                             std::is_same<T, int64_t>,
                                             std::is_same<T, uint8_t>,
                                             std::is_same<T, uint16_t>,
                                             std::is_same<T, uint32_t>,
                                             std::is_same<T, uint64_t>,
                                             std::is_same<T, float>,
                                             std::is_same<T, double>,
                                             std::is_same<T, std::string>>;

using DictValue = std::variant<bool,
                               int8_t,
                               int16_t,
                               int32_t,
                               int64_t,
                               uint8_t,
                               uint16_t,
                               uint32_t,
                               uint64_t,
                               float,
                               double,
                               std::string>;

class Dict final {
 public:
  using Self = Dict;

  Dict() noexcept = default;
  ~Dict() noexcept = default;
  CLASS_KIND_MOVABLE(Dict);

  template <typename T>
    requires IsDictValueType<T>
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
    requires IsDictValueType<T>
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
    requires IsDictValueType<T>
  [[nodiscard]] auto
  TakeOrDefault(const std::string& key, T&& default_value) noexcept -> T {
    const auto found = data_.find(key);
    if (found == data_.end()) {
      return std::move(default_value);
    }

    auto value = std::move(found->second);
    if (!std::holds_alternative<T>(value)) {
      return std::move(default_value);
    }

    data_.erase(found);
    return std::move(std::get<T>(value));
  }

  template <typename T>
    requires IsDictValueType<T>
  [[nodiscard]] auto
  TryTake(const std::string& key) noexcept -> Option<T> {
    const auto found = data_.find(key);
    if (found == data_.end()) {
      return None;
    }

    auto value = std::move(found->second);
    if (!std::holds_alternative<T>(value)) {
      return None;
    }

    data_.erase(found);
    return Option<T>{std::move(std::get<T>(value))};
  }

  template <typename T>
    requires IsDictValueType<T>
  [[nodiscard]] auto
  Set(std::string&& key, T&& value) noexcept -> Self& {
    const auto [it, inserted] =
        data_.try_emplace(std::move(key), std::move(value));
    if (!inserted) {
      assert(false && "Key already exists. Overwrite the value");
      it->second = std::move(value);
    }

    return *this;
  }

  template <typename T>
    requires IsDictValueType<T>
  [[nodiscard]] auto
  TrySet(std::string&& key, T&& value) noexcept -> bool {
    const auto [it, inserted] =
        data_.try_emplace(std::move(key), std::move(value));
    return inserted;
  }

  [[nodiscard]] auto
  Has(const std::string& key) const noexcept -> bool {
    return data_.find(key) != data_.end();
  }

  [[nodiscard]] auto
  Take() noexcept -> Self {
    return std::move(*this);
  }

 private:
  std::unordered_map<std::string, DictValue> data_;
};

}  // namespace kero

#endif  // KERO_CORE_DICT_H
