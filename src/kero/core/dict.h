#ifndef KERO_CORE_DICT_H
#define KERO_CORE_DICT_H

#include <source_location>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include "kero/core/common.h"
#include "kero/core/option.h"

namespace kero {

template <typename T>
concept IsDictValueType = std::disjunction_v<std::is_same<T, bool>,
                                             std::is_same<T, double>,
                                             std::is_same<T, std::string>>;

using DictValue = std::variant<bool, double, std::string>;

class Dict final {
 public:
  using Self = Dict;
  using Data = std::unordered_map<std::string, DictValue>;

  explicit Dict() noexcept = default;
  explicit Dict(Data&& data) noexcept;
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

  [[nodiscard]] auto
  GetOrDefault(const std::string& key, const bool default_value) const noexcept
      -> bool;

  [[nodiscard]] auto
  GetOrDefault(const std::string& key,
               const double default_value) const noexcept -> double;

  [[nodiscard]] auto
  GetOrDefault(const std::string& key,
               const std::string& default_value) const noexcept
      -> const std::string&;

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

  [[nodiscard]] auto
  TryGetAsBool(const std::string& key) const noexcept -> OptionRef<const bool&>;

  [[nodiscard]] auto
  TryGetAsDouble(const std::string& key) const noexcept
      -> OptionRef<const double&>;

  [[nodiscard]] auto
  TryGetAsString(const std::string& key) const noexcept
      -> OptionRef<const std::string&>;

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

  [[nodiscard]] auto
  TakeOrDefault(const std::string& key, bool default_value) noexcept -> bool;

  [[nodiscard]] auto
  TakeOrDefault(const std::string& key, double default_value) noexcept
      -> double;

  [[nodiscard]] auto
  TakeOrDefault(const std::string& key, std::string&& default_value) noexcept
      -> std::string;

  [[nodiscard]] auto
  TakeOrDefault(const std::string& key,
                const std::string_view default_value) noexcept -> std::string;

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

  [[nodiscard]] auto
  TryTakeAsBool(const std::string& key) noexcept -> Option<bool>;

  [[nodiscard]] auto
  TryTakeAsDouble(const std::string& key) noexcept -> Option<double>;

  [[nodiscard]] auto
  TryTakeAsString(const std::string& key) noexcept -> Option<std::string>;

  template <typename T>
    requires IsDictValueType<T>
  [[nodiscard]] auto
  Set(std::string&& key, T&& value) noexcept -> Self& {
    const auto [it, inserted] =
        data_.try_emplace(std::move(key), std::move(value));
    if (!inserted) {
      Warn("Overwriting existing data key: " + key);
      it->second = std::move(value);
    }

    return *this;
  }

  [[nodiscard]] auto
  Set(std::string&& key, bool value) noexcept -> Self&;

  [[nodiscard]] auto
  Set(std::string&& key, const double value) noexcept -> Self&;

  [[nodiscard]] auto
  Set(std::string&& key, const char* value) noexcept -> Self&;

  [[nodiscard]] auto
  Set(std::string&& key, const std::string_view value) noexcept -> Self&;

  [[nodiscard]] auto
  Set(std::string&& key, std::string&& value) noexcept -> Self&;

  template <typename T>
    requires IsDictValueType<T>
  [[nodiscard]] auto
  TrySet(std::string&& key, T&& value) noexcept -> bool {
    const auto [it, inserted] =
        data_.try_emplace(std::move(key), std::move(value));
    return inserted;
  }

  [[nodiscard]] auto
  TrySet(std::string&& key, bool value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, double value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const char* value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, const std::string_view value) noexcept -> bool;

  [[nodiscard]] auto
  TrySet(std::string&& key, std::string&& value) noexcept -> bool;

  [[nodiscard]] auto
  Remove(const std::string& key) noexcept -> Self&;

  [[nodiscard]] auto
  Has(const std::string& key) const noexcept -> bool;

  [[nodiscard]] auto
  Take() noexcept -> Self;

  [[nodiscard]] auto
  Clone() const noexcept -> Self;

  [[nodiscard]] auto
  AsRaw() const noexcept -> const Data&;

  [[nodiscard]] auto
  AsRaw() noexcept -> Data&;

 private:
  auto
  Warn(std::string&& message,
       std::source_location&& location =
           std::source_location::current()) const noexcept -> void;

  Data data_;

  friend auto
  operator<<(std::ostream& os, const Dict& dict) -> std::ostream&;
};

auto
operator<<(std::ostream& os, const Dict& dict) -> std::ostream&;

}  // namespace kero

#endif  // KERO_CORE_DICT_H
