#ifndef CPP_GAME_SERVER_PORTFOLIO_CORE_UTILS_H
#define CPP_GAME_SERVER_PORTFOLIO_CORE_UTILS_H

#include <charconv>
#include <optional>
#include <string_view>
#include <vector>

#include "core.h"

namespace core {

using Args = std::vector<std::string_view>;

[[nodiscard]] auto ParseArgcArgv(int argc, char *argv[]) noexcept -> Args;

class Tokenizer final : private NonCopyable, Movable {
public:
  explicit Tokenizer(Args &&args) noexcept;

  auto Current() const noexcept -> std::optional<std::string_view>;
  auto Next() const noexcept -> std::optional<std::string_view>;
  auto Eat() noexcept -> void;

private:
  Args args_;
  size_t index_{};
};

template <typename T>
  requires std::integral<T> || std::floating_point<T>
[[nodiscard]] auto ParseNumberString(const std::string_view token) noexcept
    -> Result<T, std::errc> {
  T value{};
  auto [ptr, ec] =
      std::from_chars(token.data(), token.data() + token.size(), value);
  if (ec != std::errc{}) {
    return ec;
  }

  return value;
}

} // namespace core

#endif // CPP_GAME_SERVER_PORTFOLIO_CORE_UTILS_H
