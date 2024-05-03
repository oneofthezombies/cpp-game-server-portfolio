#ifndef KERO_ENGINE_PIN_SYSTEM_H
#define KERO_ENGINE_PIN_SYSTEM_H

#include <mutex>
#include <unordered_map>

#include "kero/core/common.h"
#include "kero/core/result.h"
#include "kero/core/utils.h"
#include "kero/engine/pin.h"
#include "kero/engine/pin_data.h"

namespace kero {

class PinSystem final {
 public:
  using PinDataRaw = void*;
  using PinDataRawDestroyer = std::function<void(PinDataRaw)>;

  explicit PinSystem() noexcept = default;
  ~PinSystem() noexcept = default;
  CLASS_KIND_PINNABLE(PinSystem);

  auto
  DestroyAll() noexcept -> void;

  template <typename T>
  [[nodiscard]] auto
  Create(Owned<PinDataFactory<T>>&& factory) noexcept -> Result<Pin<T>> {
    return Create<T>([factory = std::move(factory)]() -> Result<T*> {
      return factory->Create();
    });
  }

  template <typename T>
  auto
  Create(PinDataFactoryFn<T>&& factory_fn) noexcept -> Result<Pin<T>> {
    using ResultT = Result<Pin<T>>;

    auto factory_res = factory_fn();
    if (factory_res.IsErr()) {
      return ResultT::Err(factory_res.TakeErr());
    }

    auto pin_data = factory_res.TakeOk();
    if (pin_data == nullptr) {
      return ResultT::Err(
          Json{}.Set("message", "factory must not return nullptr").Take());
    }

    Defer delete_pin_data{[pin_data] { delete pin_data; }};
    auto pin_data_raw = static_cast<PinDataRaw>(pin_data);

    {
      std::lock_guard lock{mutex_};
      auto found = pin_map_.find(pin_data_raw);
      if (found != pin_map_.end()) {
        return ResultT::Err(Error::From(
            Json{}
                .Set("message", "factory returned duplicate pin data")
                .Take()));
      }

      pin_map_.emplace(pin_data, [](PinDataRaw pin_data_raw) {
        delete static_cast<T*>(pin_data_raw);
      });
    }

    delete_pin_data.Cancel();
    return ResultT::Ok(Pin<T>{pin_data});
  }

 private:
  std::unordered_map<PinDataRaw, PinDataRawDestroyer> pin_map_{};
  std::mutex mutex_{};
};

}  // namespace kero

#endif  // KERO_ENGINE_PIN_SYSTEM_H
