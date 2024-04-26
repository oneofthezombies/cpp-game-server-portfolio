#ifndef CORE_TINY_JSON_H
#define CORE_TINY_JSON_H

#include <iostream>
#include <optional>
#include <source_location>
#include <sstream>
#include <string_view>
#include <unordered_map>

#include "core.h"

class TinyJson final {
public:
  using Raw = std::unordered_map<std::string, std::string>;

  explicit TinyJson() noexcept = default;
  explicit TinyJson(Raw &&raw) noexcept;
  ~TinyJson() noexcept = default;
  CLASS_KIND_MOVABLE(TinyJson);

  /**
   * Returning value is a reference to the value in the map.
   */
  [[nodiscard]] auto Get(const std::string_view key) const noexcept
      -> std::optional<std::string_view>;

  template <typename T>
  auto Set(const std::string_view key, const T &value) noexcept -> TinyJson & {
    std::ostringstream oss;
    oss << value;
    auto key_str = std::string(key);
    auto found = raw_.find(key_str);
    if (found != raw_.end()) {
      std::cout << "TinyJson duplicated key: " << key_str << ", "
                << "Old value: " << found->second << ", "
                << "New value: " << oss.str() << "\n";
      found->second = oss.str();
    } else {
      raw_.emplace(std::move(key_str), oss.str());
    }
    return *this;
  }

  auto AsRaw() const noexcept -> const Raw &;

  [[nodiscard]] auto Clone() const noexcept -> TinyJson;

  [[nodiscard]] auto ToString() const noexcept -> std::string;

  [[nodiscard]] static auto Parse(const std::string_view tiny_json_str) noexcept
      -> std::optional<TinyJson>;

private:
  Raw raw_;

  friend class TinyJsonParser;
};

auto operator<<(std::ostream &os, const TinyJson &tiny_json) noexcept
    -> std::ostream &;

struct TinyJsonParserOptions {
  bool allow_trailing_comma{false};

  explicit TinyJsonParserOptions() noexcept = default;
  ~TinyJsonParserOptions() noexcept = default;
  CLASS_KIND_COPYABLE(TinyJsonParserOptions);
};

class TinyJsonParser final {
public:
  explicit TinyJsonParser(
      const std::string_view tiny_json_str,
      TinyJsonParserOptions &&options = TinyJsonParserOptions{}) noexcept;
  ~TinyJsonParser() noexcept = default;
  CLASS_KIND_MOVABLE(TinyJsonParser);

  [[nodiscard]] auto Parse() noexcept -> std::optional<TinyJson>;

private:
  [[nodiscard]] auto ParseKeyValue() noexcept
      -> std::optional<std::pair<std::string, std::string>>;
  [[nodiscard]] auto ParseKey() noexcept -> std::optional<std::string>;
  [[nodiscard]] auto ParseValue() noexcept -> std::optional<std::string>;
  [[nodiscard]] auto ParseString() noexcept -> std::optional<std::string>;

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

  std::string_view tiny_json_str_;
  size_t cursor_{};
  TinyJson::Raw raw_;
  TinyJsonParserOptions options_;
};

#endif // CORE_TINY_JSON_H
