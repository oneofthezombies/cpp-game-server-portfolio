#ifndef KERO_ENGINE_PINNING_SYSTEM_H
#define KERO_ENGINE_PINNING_SYSTEM_H

#include <functional>
#include <mutex>
#include <unordered_map>

#include "kero/core/common.h"
#include "kero/core/result.h"
#include "kero/core/utils.h"

namespace kero {

template <typename T>
using Pinned = T*;

template <typename T>
class PinnedFactory {
 public:
  explicit PinnedFactory() noexcept = default;
  virtual ~PinnedFactory() noexcept = default;
  CLASS_KIND_PINNABLE(PinnedFactory);

  [[nodiscard]] virtual auto
  Create() noexcept -> Result<Pinned<T>> = 0;
};

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
  Create(Owned<PinnedFactory<T>>&& factory) noexcept -> Result<Pinned<T>> {
    using ResultT = Result<Pinned<T>>;

    auto res = factory();
    if (res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    T* ptr = res.TakeOk();
    if (ptr == nullptr) {
      return ResultT::Err(
          Dict{}.Set("message", "factory must not return nullptr").Take());
    }

    Defer delete_ptr{[ptr] { delete ptr; }};
    Raw raw = static_cast<Raw>(ptr);

    {
      std::lock_guard lock{mutex_};
      auto found = pinned_map_.find(raw);
      if (found != pinned_map_.end()) {
        return ResultT::Err(Error::From(
            Dict{}
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
