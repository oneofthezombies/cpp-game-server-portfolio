#ifndef CPP_GAME_SERVER_PORTFOLIO_CORE_RESULT_H
#define CPP_GAME_SERVER_PORTFOLIO_CORE_RESULT_H

#include <variant>

namespace core {

template <typename T, typename E> class Result {
public:
  Result(T &&value) : data_(std::forward<T>(value)) {}
  Result(E &&error) : data_(std::forward<E>(error)) {}

  auto IsOk() const noexcept -> bool {
    return std::holds_alternative<T>(data_);
  }
  auto IsErr() const noexcept -> bool {
    return std::holds_alternative<E>(data_);
  }

  auto Ok() noexcept -> T & { return std::get<T>(data_); }
  auto Err() noexcept -> E & { return std::get<E>(data_); }

private:
  std::variant<std::monostate, T, E> data_;
};

} // namespace core

#endif // CPP_GAME_SERVER_PORTFOLIO_CORE_RESULT_H
