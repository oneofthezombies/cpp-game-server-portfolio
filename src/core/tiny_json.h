#ifndef CORE_TINY_JSON_H
#define CORE_TINY_JSON_H

#include <optional>
#include <source_location>
#include <sstream>
#include <string_view>
#include <unordered_map>

#include "core.h"

namespace core {

class TinyJson final : private NonCopyable, Movable {
public:
  [[nodiscard]] auto Get(const std::string_view key) const noexcept
      -> std::optional<std::string_view>;

  auto AsRaw() const noexcept
      -> const std::unordered_map<std::string_view, std::string_view> &;

  /**
   * The lifetime in the input `tiny_json` must be longer than the lifetime
   * in `std::string_view` returned by the `TinyJson` object and the object's
   * `Get` method.
   */
  [[nodiscard]] static auto Parse(const std::string_view tiny_json) noexcept
      -> std::optional<TinyJson>;

private:
  explicit TinyJson(
      std::unordered_map<std::string_view, std::string_view> &&data) noexcept;

  std::unordered_map<std::string_view, std::string_view> data_;

  friend class TinyJsonParser;
};

auto operator<<(std::ostream &os, const TinyJson &tiny_json) noexcept
    -> std::ostream &;

class TinyJsonParser final : private NonCopyable, Movable {
public:
  explicit TinyJsonParser(const std::string_view tiny_json) noexcept;

  [[nodiscard]] auto Parse() noexcept -> std::optional<TinyJson>;

private:
  [[nodiscard]] auto ParseKeyValue() noexcept
      -> std::optional<std::pair<std::string_view, std::string_view>>;
  [[nodiscard]] auto ParseKey() noexcept -> std::optional<std::string_view>;
  [[nodiscard]] auto ParseValue() noexcept -> std::optional<std::string_view>;
  [[nodiscard]] auto ParseString() noexcept -> std::optional<std::string_view>;

  [[nodiscard]] auto Current(const std::source_location location =
                                 std::source_location::current()) const noexcept
      -> std::optional<char>;
  [[nodiscard]] auto Consume(const char c,
                             const std::source_location location =
                                 std::source_location::current()) noexcept
      -> bool;
  [[nodiscard]] auto Advance(const std::source_location location =
                                 std::source_location::current()) noexcept
      -> bool;
  auto Trim() noexcept -> void;

  auto Log(const std::string_view message,
           const std::source_location location) const noexcept -> void;

  std::string_view tiny_json_;
  size_t cursor_{};
  std::unordered_map<std::string_view, std::string_view> data_{};
};

class TinyJsonBuilder final : private NonCopyable, NonMovable {
public:
  template <typename T>
  auto Add(std::string &&key, const T &value) noexcept -> TinyJsonBuilder & {
    std::ostringstream oss;
    oss << value;
    data_.emplace(key, oss.str());
    return *this;
  }

  [[nodiscard]] auto Build() noexcept -> std::string;

private:
  std::unordered_map<std::string, std::string> data_{};
};

} // namespace core

#endif // CORE_TINY_JSON_H
