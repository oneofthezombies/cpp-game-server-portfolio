#ifndef KERO_ENGINE_PINNING_SYSTEM_H
#define KERO_ENGINE_PINNING_SYSTEM_H

#include <functional>
#include <mutex>
#include <unordered_map>

#include "kero/core/common.h"
#include "kero/core/result.h"
#include "kero/core/utils.h"
#include "kero/engine/pinned.h"

namespace kero {

class PinningSystem final {
 public:
  using Raw = void*;
  using RawDestroyer = std::function<void(Raw)>;

  explicit PinningSystem() noexcept = default;
  ~PinningSystem() noexcept = default;
  CLASS_KIND_PINNABLE(PinningSystem);

  auto
  DestroyAll() noexcept -> void;

  auto
  Destroy(Raw raw) noexcept -> void;

  template <typename T>
  [[nodiscard]] auto
  Register(T* ptr) noexcept -> Result<Pinned<T>> {
    using ResultT = Result<Pinned<T>>;

    if (ptr == nullptr) {
      return ResultT::Err(
          Json{}.Set("message", "factory must not return nullptr").Take());
    }

    Defer delete_ptr{[ptr] { delete ptr; }};
    Raw raw = static_cast<Raw>(ptr);

    {
      std::lock_guard lock{mutex_};
      auto found = pinned_map_.find(raw);
      if (found != pinned_map_.end()) {
        return ResultT::Err(Error::From(
            Json{}
                .Set("message", "factory returned duplicate pointer")
                .Take()));
      }

      pinned_map_.emplace(raw, [](Raw raw) { delete static_cast<T*>(raw); });
    }

    delete_ptr.Cancel();
    return ResultT::Ok(Pinned<T>{ptr});
  }

 private:
  std::unordered_map<Raw, RawDestroyer> pinned_map_{};
  std::mutex mutex_{};
};

}  // namespace kero

#endif  // KERO_ENGINE_PINNING_SYSTEM_H
