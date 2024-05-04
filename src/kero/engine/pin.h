#ifndef KERO_ENGINE_PIN_H
#define KERO_ENGINE_PIN_H

#include <cassert>

#include "kero/core/common.h"

namespace kero {

template <typename T>
class Pin final {
 public:
  ~Pin() noexcept = default;
  KERO_CLASS_KIND_COPYABLE(Pin);

  [[nodiscard]] operator bool() const noexcept { return data_ != nullptr; }

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
    return data_;
  }

 private:
  explicit Pin(T* data) noexcept : data_{data} {
    assert(data_ != nullptr && "Pin data must not be null");
  }

  T* data_{};

  friend class PinSystem;
};

}  // namespace kero

#endif  // KERO_ENGINE_PIN_H
