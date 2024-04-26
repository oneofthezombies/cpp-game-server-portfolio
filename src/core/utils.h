#ifndef CORE_UTILS_H
#define CORE_UTILS_H

#include <charconv>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

#include "core.h"

namespace core {

using Args = std::vector<std::string_view>;

[[nodiscard]] auto ParseArgcArgv(int argc, char *argv[]) noexcept -> Args;

class Tokenizer final {
public:
  explicit Tokenizer(Args &&args) noexcept;
  ~Tokenizer() noexcept = default;
  CLASS_KIND_MOVABLE(Tokenizer);

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
    -> ResultBase<T, std::errc> {
  using ResultT = ResultBase<T, std::errc>;

  T value{};
  auto [ptr, ec] =
      std::from_chars(token.data(), token.data() + token.size(), value);
  if (ec != std::errc{}) {
    return ResultT{ec};
  }

  return ResultT{value};
}

class Defer final {
public:
  explicit Defer(std::function<void()> &&fn) noexcept;
  ~Defer() noexcept;
  CLASS_KIND_PINNABLE(Defer);

  auto Cancel() noexcept -> void;

private:
  std::function<void()> fn_;
};

} // namespace core

#endif // CORE_UTILS_H
