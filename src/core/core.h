#ifndef CORE_CORE_H
#define CORE_CORE_H

#include <memory>
#include <ostream>
#include <source_location>
#include <string>
#include <unordered_map>
#include <variant>

#define CLASS_KIND_COPYABLE(cls)                           \
  cls(const cls &) noexcept = default;                     \
  cls(cls &&) noexcept = default;                          \
  auto operator=(const cls &) noexcept -> cls & = default; \
  auto operator=(cls &&) noexcept -> cls & = default

#define CLASS_KIND_MOVABLE(cls)                \
  cls(const cls &) = delete;                   \
  cls(cls &&) noexcept = default;              \
  auto operator=(const cls &)->cls & = delete; \
  auto operator=(cls &&) noexcept -> cls & = default

#define CLASS_KIND_PINNABLE(cls)               \
  cls(const cls &) = delete;                   \
  cls(cls &&) = delete;                        \
  auto operator=(const cls &)->cls & = delete; \
  auto operator=(cls &&)->cls & = delete

namespace core {

struct Void final {
  explicit Void() noexcept = default;
  ~Void() noexcept = default;
  CLASS_KIND_COPYABLE(Void);
};

auto
operator<<(std::ostream &os, const Void &) -> std::ostream &;

using ErrorCode = int32_t;
using ErrorDetails = std::unordered_map<std::string, std::string>;

auto
DebugErrorDetails(std::ostream &os, const ErrorDetails &details) -> void;

struct Error;
using ErrorCause = std::unique_ptr<Error>;

struct Error final {
  ErrorCode code;
  ErrorDetails details;
  ErrorCause cause;
  std::source_location location;

  explicit Error(const ErrorCode code,
                 ErrorDetails &&details,
                 ErrorCause &&cause,
                 std::source_location &&location) noexcept;

  ~Error() noexcept = default;
  CLASS_KIND_MOVABLE(Error);

  static auto
  From(const ErrorCode code,
       ErrorDetails &&details = {},
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;

  static auto
  From(Error &&cause,
       std::source_location &&location =
           std::source_location::current()) noexcept -> Error;
};

auto
operator<<(std::ostream &os, const Error &error) -> std::ostream &;

template <typename T, typename E>
class ResultBase final {
 public:
  explicit ResultBase(const T &value) : data_(std::in_place_type<T>, value) {}
  explicit ResultBase(T &&value)
      : data_(std::in_place_type<T>, std::forward<T>(value)) {}
  explicit ResultBase(E &&error)
      : data_(std::in_place_type<E>, std::forward<E>(error)) {}
  ~ResultBase() noexcept = default;
  CLASS_KIND_MOVABLE(ResultBase);

  auto
  IsOk() const noexcept -> bool {
    return std::holds_alternative<T>(data_);
  }

  auto
  IsErr() const noexcept -> bool {
    return std::holds_alternative<E>(data_);
  }

  [[nodiscard]] auto
  Ok() const noexcept -> const T & {
    return std::get<T>(data_);
  }

  [[nodiscard]] auto
  Ok() noexcept -> T & {
    return std::get<T>(data_);
  }

  [[nodiscard]] auto
  Err() const noexcept -> const E & {
    return std::get<E>(data_);
  }

  [[nodiscard]] auto
  Err() noexcept -> E & {
    return std::get<E>(data_);
  }

 private:
  std::variant<std::monostate, T, E> data_;
};

template <typename T, typename E>
auto
operator<<(std::ostream &os, const ResultBase<T, E> &result) -> std::ostream & {
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
using Result = ResultBase<T, Error>;

}  // namespace core

#endif  // CORE_CORE_H
