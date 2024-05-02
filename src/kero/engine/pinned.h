#ifndef KERO_ENGINE_PINNED_H
#define KERO_ENGINE_PINNED_H

#include <cassert>

#include "kero/core/common.h"

namespace kero {

template <typename T>
class Pinned final {
 public:
  explicit Pinned(T* ptr) noexcept : ptr_{ptr} {
    assert(ptr_ != nullptr && "Pinned pointer must not be null");
  }

  ~Pinned() noexcept = default;
  CLASS_KIND_COPYABLE(Pinned);

  [[nodiscard]] operator bool() const noexcept { return ptr_ != nullptr; }

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

#endif  // KERO_ENGINE_PINNED_H
