#ifndef SERVER_SESSION_H
#define SERVER_SESSION_H

#include <memory>

#include "core/core.h"

class SessionImplDeleter final : private NonCopyable, Movable {
public:
  auto operator()(void *impl_raw) const noexcept -> void;
};

using SessionImplPtr = std::unique_ptr<void, SessionImplDeleter>;

enum class SessionState : int32_t {
  kCreated = 0,
};

class Session final : private NonCopyable, Movable {
public:
  using IdType = uint64_t;

  explicit Session(SessionImplPtr &&impl) noexcept;

  [[nodiscard]] auto Id() const noexcept -> IdType;

  [[nodiscard]] auto Hash() const noexcept -> size_t;

private:
  SessionImplPtr impl_;
  SessionState state_{SessionState::kCreated};
};

auto operator==(const Session &lhs, const Session &rhs) noexcept -> bool;

auto operator<<(std::ostream &os, const Session &session) -> std::ostream &;

namespace std {

template <> struct hash<Session> {
  auto operator()(const Session &session) const noexcept -> size_t {
    return session.Hash();
  }
};

} // namespace std

#endif // SERVER_SESSION_H
