#ifndef KERO_CORE_COMMON_H
#define KERO_CORE_COMMON_H

#include <memory>

#define CLASS_KIND_COPYABLE(cls)                           \
  cls(const cls &) noexcept = default;                     \
  cls(cls &&) noexcept = default;                          \
  auto operator=(const cls &) noexcept -> cls & = default; \
  auto operator=(cls &&) noexcept -> cls & = default

#define CLASS_KIND_MOVABLE(cls)                  \
  cls(const cls &) = delete;                     \
  cls(cls &&) noexcept = default;                \
  auto operator=(const cls &) -> cls & = delete; \
  auto operator=(cls &&) noexcept -> cls & = default

#define CLASS_KIND_PINNABLE(cls)                 \
  cls(const cls &) = delete;                     \
  cls(cls &&) = delete;                          \
  auto operator=(const cls &) -> cls & = delete; \
  auto operator=(cls &&) -> cls & = delete

namespace kero {

struct Void final {
  explicit Void() noexcept = default;
  ~Void() noexcept = default;
  CLASS_KIND_COPYABLE(Void);
};

template <typename T>
using Owned = std::unique_ptr<T>;

template <typename T>
using Borrowed = T *;

}  // namespace kero

#endif  // KERO_CORE_COMMON_H
