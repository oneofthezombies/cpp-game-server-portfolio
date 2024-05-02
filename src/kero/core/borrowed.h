#ifndef KERO_CORE_BORROWED_H
#define KERO_CORE_BORROWED_H

#include <cassert>

#include "kero/core/common.h"

namespace kero {

template <typename T>
class Borrowed final {
 public:
  explicit Borrowed(T* ptr) noexcept : ptr_{ptr} {
    assert(ptr_ != nullptr && "Borrowed pointer must not be null");
  }

  ~Borrowed() noexcept = default;
  CLASS_KIND_COPYABLE(Borrowed);

  [[nodiscard]] auto
  operator->() const noexcept -> T* {
    return Get();
  }

  [[nodiscard]] auto
  operator*() const noexcept -> T const& {
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
