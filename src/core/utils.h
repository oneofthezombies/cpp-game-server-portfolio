#ifndef CORE_UTILS_H
#define CORE_UTILS_H

#include <charconv>
#include <functional>
#include <optional>
#include <sstream>
#include <string_view>
#include <vector>

#include "core.h"

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

class StringBuilder final {
public:
  explicit StringBuilder() noexcept = default;
  ~StringBuilder() noexcept = default;
  CLASS_KIND_MOVABLE(StringBuilder);

  template <typename T>
  [[nodiscard]] auto Add(const T &value) noexcept -> StringBuilder & {
    oss_ << value;
    return *this;
  }

  template <typename T>
  [[nodiscard]] auto Add(const std::string_view key, const T &value) noexcept
      -> StringBuilder & {
    oss_ << key << "=" << value;
    return *this;
  }

  [[nodiscard]] auto Build() const noexcept -> std::string {
    return oss_.str();
  }

private:
  std::ostringstream oss_;
};

class Defer final {
public:
  explicit Defer(std::function<void()> &&fn) noexcept;
  ~Defer() noexcept;
  CLASS_KIND_PINNABLE(Defer);

  auto Cancel() noexcept -> void;

private:
  std::function<void()> fn_;
};

#endif // CORE_UTILS_H
