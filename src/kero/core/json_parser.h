#ifndef KERO_CORE_JSON_PARSER_H
#define KERO_CORE_JSON_PARSER_H

#include "kero/core/common.h"
#include "kero/core/json.h"
#include "kero/core/result.h"

namespace kero {

class JsonParser final {
 public:
  struct ParseOptions {
    bool allow_trailing_comma{false};

    static auto
    Default() noexcept -> ParseOptions {
      return ParseOptions{};
    }
  };

  explicit JsonParser() noexcept = default;
  ~JsonParser() noexcept = default;
  CLASS_KIND_PINNABLE(JsonParser);

  [[nodiscard]] auto
  Parse(const std::string_view tiny_json_str,
        ParseOptions &&options = ParseOptions::Default()) noexcept
      -> Result<Json>;

  [[nodiscard]] static auto
  Stringify(const Json &json) noexcept -> Result<std::string>;

 private:
  [[nodiscard]] auto
  ParseObject() noexcept -> Result<Json>;

  [[nodiscard]] auto
  ParseKey() noexcept -> Result<std::string>;

  [[nodiscard]] auto
  ParseValue() noexcept -> Result<Json::ValueStorage>;

  [[nodiscard]] auto
  ParseString() noexcept -> Result<std::string>;

  [[nodiscard]] auto
  ParseNumber() noexcept -> Result<double>;

  [[nodiscard]] auto
  ParseBool() noexcept -> Result<bool>;

  [[nodiscard]] auto
  Current() const noexcept -> Result<char>;

  [[nodiscard]] auto
  Consume(const char c) noexcept -> Result<bool>;

  [[nodiscard]] auto
  Advance(const size_t n = 1) noexcept -> Result<bool>;

  auto
  Trim() noexcept -> void;

  std::string_view tiny_json_str_{};
  size_t cursor_{};
  ParseOptions options_{};
};

}  // namespace kero

#endif  // KERO_CORE_JSON_PARSER_H
