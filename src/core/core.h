#ifndef CPP_GAME_SERVER_PORTFOLIO_CORE_CORE_H
#define CPP_GAME_SERVER_PORTFOLIO_CORE_CORE_H

#include <ostream>
#include <string>
#include <variant>

#define CLASS_KIND_COPYABLE(cls)                                               \
  cls(const cls &) noexcept = default;                                         \
  cls(cls &&) noexcept = default;                                              \
  auto operator=(const cls &) noexcept -> cls & = default;                     \
  auto operator=(cls &&) noexcept -> cls & = default

#define CLASS_KIND_MOVABLE(cls)                                                \
  cls(const cls &) = delete;                                                   \
  cls(cls &&) noexcept = default;                                              \
  auto operator=(const cls &) -> cls & = delete;                               \
  auto operator=(cls &&) noexcept -> cls & = default

#define CLASS_KIND_PINNABLE(cls)                                               \
  cls(const cls &) = delete;                                                   \
  cls(cls &&) = delete;                                                        \
  auto operator=(const cls &) -> cls & = delete;                               \
  auto operator=(cls &&) -> cls & = delete

template <typename T, typename E> class ResultBase final {
public:
  explicit ResultBase(const T &value) : data_(value) {}
  explicit ResultBase(T &&value) : data_(std::forward<T>(value)) {}
  explicit ResultBase(const E &error) : data_(error) {}
  explicit ResultBase(E &&error) : data_(std::forward<E>(error)) {}
  ~ResultBase() noexcept = default;
  CLASS_KIND_MOVABLE(ResultBase);

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

template <typename T, typename E>
auto operator<<(std::ostream &os, const ResultBase<T, E> &result)
    -> std::ostream & {
  os << "Result{";
  if (result.IsOk()) {
    os << "Ok{";
    os << result.Ok();
    os << "}";
  } else if (result.IsErr()) {
    os << "Err{";
    os << result.Err();
    os << "}";
  } else {
    os << "Empty{}";
  }
  os << "}";
  return os;
}

template <typename T>
  requires std::is_enum_v<T>
struct ErrorBase final {
  T code;
  std::string message;

  explicit ErrorBase(const T code) noexcept : code(code) {}
  explicit ErrorBase(const T code, std::string &&message) noexcept
      : code(code), message(std::move(message)) {}
  ~ErrorBase() noexcept = default;
  CLASS_KIND_MOVABLE(ErrorBase);
};

template <typename T>
auto operator<<(std::ostream &os, const ErrorBase<T> &error) -> std::ostream & {
  os << "Error{";
  os << "code=";
  os << error.code;
  os << ", ";
  os << "message=";
  os << error.message;
  os << "}";
  return os;
}

struct Void final {
  explicit Void() noexcept = default;
  ~Void() noexcept = default;
  CLASS_KIND_COPYABLE(Void);
};

auto operator<<(std::ostream &os, const Void &) -> std::ostream &;

#endif // CPP_GAME_SERVER_PORTFOLIO_CORE_CORE_H
