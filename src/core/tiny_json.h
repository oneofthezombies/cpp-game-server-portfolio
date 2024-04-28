#ifndef CORE_TINY_JSON_H
#define CORE_TINY_JSON_H

#include <iostream>
#include <optional>
#include <ostream>
#include <source_location>
#include <sstream>
#include <string_view>
#include <unordered_map>

#include "core.h"
#include "core/utils.h"

namespace core {

class TinyJson final {
 public:
  using Map = std::unordered_map<std::string, std::string>;

  explicit TinyJson() noexcept = default;
  explicit TinyJson(Map &&raw) noexcept;
  ~TinyJson() noexcept = default;
  CLASS_KIND_MOVABLE(TinyJson);

  /**
   * Returning value is a reference to the value in the map.
   */
  [[nodiscard]] auto
  Get(const std::string_view key) const noexcept -> Result<std::string_view>;

  template <typename T>
    requires std::integral<T> || std::floating_point<T>
  [[nodiscard]] auto
  GetAsNumber(const std::string_view key) const noexcept -> Result<T> {
    using ResultT = Result<T>;

    auto get_res = Get(key);
    if (get_res.IsErr()) {
      return ResultT{Error::From(get_res.TakeErr())};
    }

    auto parse_res = core::ParseNumberString<T>(get_res.Ok());
    if (parse_res.IsErr()) {
      return ResultT{Error::From(parse_res.TakeErr())};
    }

    return ResultT{parse_res.Ok()};
  }

  template <typename T>
  auto
  Set(const std::string_view key, const T &value) noexcept -> TinyJson & {
    std::ostringstream oss;
    oss << value;
    auto key_str = std::string(key);
    auto found = map_.find(key_str);
    if (found != map_.end()) {
      std::cout << "TinyJson duplicated key: " << key_str << ", "
                << "Old value: " << found->second << ", "
                << "New value: " << oss.str() << "\n";
      found->second = oss.str();
    } else {
      map_.emplace(std::move(key_str), oss.str());
    }
    return *this;
  }

  auto
  AsMap() const noexcept -> const Map &;

  /**
   * This method will move the ownership of the raw data to the caller.
   */
  [[nodiscard]] auto
  IntoMap() noexcept -> Map;

  [[nodiscard]] auto
  Clone() const noexcept -> TinyJson;

  [[nodiscard]] auto
  ToString() const noexcept -> std::string;

  auto
  Log(std::ostream &os = std::cout,
      std::source_location location =
          std::source_location::current()) const noexcept -> void;

  auto
  LogLn(std::ostream &os = std::cout,
        std::source_location location =
            std::source_location::current()) const noexcept -> void;

  [[nodiscard]] static auto
  Parse(const std::string_view tiny_json_str) noexcept
      -> std::optional<TinyJson>;

 private:
  Map map_;

  friend class TinyJsonParser;
};

auto
operator<<(std::ostream &os,
           const TinyJson &tiny_json) noexcept -> std::ostream &;

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

  [[nodiscard]] auto
  Parse() noexcept -> std::optional<TinyJson>;

 private:
  [[nodiscard]] auto
  ParseKeyValue() noexcept
      -> std::optional<std::pair<std::string, std::string>>;
  [[nodiscard]] auto
  ParseKey() noexcept -> std::optional<std::string>;
  [[nodiscard]] auto
  ParseValue() noexcept -> std::optional<std::string>;
  [[nodiscard]] auto
  ParseString() noexcept -> std::optional<std::string>;

  [[nodiscard]] auto
  Current(const std::source_location location = std::source_location::current())
      const noexcept -> std::optional<char>;
  [[nodiscard]] auto
  Consume(const char c,
          const std::source_location location =
              std::source_location::current()) noexcept -> bool;
  [[nodiscard]] auto
  Advance(const std::source_location location =
              std::source_location::current()) noexcept -> bool;
  auto
  Trim() noexcept -> void;

  auto
  Log(const std::string_view message,
      const std::source_location location) const noexcept -> void;

  std::string_view tiny_json_str_;
  size_t cursor_{};
  TinyJson::Map map_;
  TinyJsonParserOptions options_;
};

}  // namespace core

#endif  // CORE_TINY_JSON_H
