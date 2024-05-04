#ifndef KERO_CORE_BORROWED_H
#define KERO_CORE_BORROWED_H

#include <cassert>

#include "kero/core/common.h"

namespace kero {

template <typename T>
class Borrow final {
 public:
  explicit Borrow(const Own<T>& own) noexcept : ptr_{own.get()} {
    assert(ptr_ != nullptr && "Borrow pointer must not be null");
  }

  ~Borrow() noexcept = default;
  CLASS_KIND_COPYABLE(Borrow);

  [[nodiscard]] auto
  operator->() const noexcept -> T* {
    return Get();
  }

  [[nodiscard]] auto
  operator*() const noexcept -> T& {
    return *Get();
  }

  [[nodiscard]] auto
  Get() const noexcept -> T* {
    return ptr_;
  }

 private:
  T* ptr_{};
};

}  // namespace kero

#endif  // KERO_CORE_BORROWED_H
