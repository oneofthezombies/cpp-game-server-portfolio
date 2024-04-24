#ifndef CPP_GAME_SERVER_PORTFOLIO_CORE_CORE_H
#define CPP_GAME_SERVER_PORTFOLIO_CORE_CORE_H

#include <string>
#include <variant>

namespace core {

class Copyable {
public:
  Copyable() noexcept = default;
  Copyable(const Copyable &) noexcept = default;
  auto operator=(const Copyable &) noexcept -> Copyable & = default;
};

class NonCopyable {
public:
  NonCopyable() noexcept = default;
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable(NonCopyable &&) noexcept = default;
  auto operator=(const NonCopyable &) -> NonCopyable & = delete;
  auto operator=(NonCopyable &&) noexcept -> NonCopyable & = default;
};

class Movable {
public:
  Movable() noexcept = default;
  Movable(Movable &&) noexcept = default;
  auto operator=(Movable &&) noexcept -> Movable & = default;
};

class NonMovable {
public:
  NonMovable() noexcept = default;
  NonMovable(const NonMovable &) noexcept = default;
  NonMovable(NonMovable &&) = delete;
  auto operator=(const NonMovable &) -> NonMovable & = default;
  auto operator=(NonMovable &&) -> NonMovable & = delete;
};

template <typename T, typename E>
  requires std::movable<T> && std::movable<E>
class Result final : private NonCopyable, Movable {
public:
  Result(const T &value) : data_(value) {}
  Result(T &&value) : data_(std::forward<T>(value)) {}
  Result(const E &error) : data_(error) {}
  Result(E &&error) : data_(std::forward<E>(error)) {}

  auto IsOk() const noexcept -> bool {
    return std::holds_alternative<T>(data_);
  }

  auto IsErr() const noexcept -> bool {
    return std::holds_alternative<E>(data_);
  }

  [[nodiscard]] auto Ok() const noexcept -> const T & {
    return std::get<T>(data_);
  }

  [[nodiscard]] auto Ok() noexcept -> T & { return std::get<T>(data_); }

  [[nodiscard]] auto Err() const noexcept -> const E & {
    return std::get<E>(data_);
  }

  [[nodiscard]] auto Err() noexcept -> E & { return std::get<E>(data_); }

private:
  std::variant<std::monostate, T, E> data_;
};

template <typename T>
  requires std::is_enum_v<T>
struct Error final : private NonCopyable, Movable {
  T code;
  std::string message;

  Error(const T code) noexcept : code(code) {}
  Error(const T code, std::string &&message) noexcept
      : code(code), message(std::move(message)) {}
};

struct Void final : private Copyable, Movable {};

auto operator<<(std::ostream &os, const Void &) -> std::ostream &;

} // namespace core

#endif // CPP_GAME_SERVER_PORTFOLIO_CORE_CORE_H
