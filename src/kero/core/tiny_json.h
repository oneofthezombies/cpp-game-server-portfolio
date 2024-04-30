#ifndef KERO_CORE_TINY_JSON_H
#define KERO_CORE_TINY_JSON_H

#include "kero/core/common.h"
#include "kero/core/dict.h"
#include "kero/core/result.h"

namespace kero {

class TinyJson final {
 public:
  struct ParseOptions {
    bool allow_trailing_comma{false};

    static auto
    Default() noexcept -> ParseOptions {
      return ParseOptions{};
    }
  };

  explicit TinyJson() noexcept = default;
  ~TinyJson() noexcept = default;
  CLASS_KIND_PINNABLE(TinyJson);

  [[nodiscard]] auto
  Parse(const std::string_view tiny_json_str,
        ParseOptions &&options = ParseOptions::Default()) noexcept
      -> Result<Dict>;

  [[nodiscard]] static auto
  Stringify(const Dict &dict) noexcept -> Result<std::string>;

 private:
  [[nodiscard]] auto
  ParseObject() noexcept -> Result<Dict>;

  [[nodiscard]] auto
  ParseKey() noexcept -> Result<std::string>;

  [[nodiscard]] auto
  ParseValue() noexcept -> Result<DictValue>;

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

#endif  // KERO_CORE_TINY_JSON_H
