#ifndef KERO_ENGINE_PIN_OBJECT_SYSTEM_H
#define KERO_ENGINE_PIN_OBJECT_SYSTEM_H

#include <cassert>
#include <functional>
#include <mutex>
#include <unordered_map>

#include "kero/core/common.h"
#include "kero/core/result.h"
#include "kero/core/utils.h"

namespace kero {

template <typename T>
class Pin final {
 public:
  explicit Pin(T* data) noexcept : data_{data} {
    assert(data_ != nullptr && "data must not be nullptr");
  }

  ~Pin() noexcept = default;
  CLASS_KIND_COPYABLE(Pin);

  [[nodiscard]] auto
  operator->() noexcept -> T* {
    return data_;
  }

  [[nodiscard]] auto
  operator->() const noexcept -> const T* {
    return data_;
  }

  [[nodiscard]] auto
  operator*() noexcept -> T& {
    return Unwrap();
  }

  [[nodiscard]] auto
  operator*() const noexcept -> const T& {
    return Unwrap();
  }

  [[nodiscard]] auto
  Unwrap() noexcept -> T& {
    return *data_;
  }

  [[nodiscard]] auto
  Unwrap() const noexcept -> const T& {
    return *data_;
  }

 private:
  T* data_{};
};

template <typename T>
using PinObjectFactory = std::function<Result<T*>()>;

class PinObjectSystem final {
 public:
  using Raw = void*;
  using RawDeleter = std::function<void(Raw)>;

  explicit PinObjectSystem() noexcept = default;
  ~PinObjectSystem() noexcept = default;
  CLASS_KIND_PINNABLE(PinObjectSystem);

  auto
  DeleteAll() noexcept -> void;

  template <typename T>
  [[nodiscard]] auto
  CreatePinObject(PinObjectFactory<T>&& factory) noexcept -> Result<Pin<T>> {
    using ResultT = Result<Pin<T>>;

    auto res = factory();
    if (res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }

    T* ptr = res.TakeOk();
    if (ptr == nullptr) {
      return ResultT::Err(Error::From(
          Dict{}.Set("message", "factory returned nullptr").Take()));
    }

    Defer delete_ptr{[ptr] { delete ptr; }};

    {
      std::lock_guard lock{mutex_};
      auto it = pin_objects_.try_emplace(static_cast<Raw>(ptr), [](Raw raw) {
        delete static_cast<T*>(raw);
      });

      if (!it.second) {
        return ResultT::Err(Error::From(
            Dict{}
                .Set("message", "factory returned duplicate pointer")
                .Take()));
      }
    }

    delete_ptr.Cancel();
    return ResultT::Ok(Pin<T>{ptr});
  }

 private:
  std::unordered_map<Raw, RawDeleter> pin_objects_{};
  std::mutex mutex_{};
};

}  // namespace kero

#endif  // KERO_ENGINE_PIN_OBJECT_SYSTEM_H
