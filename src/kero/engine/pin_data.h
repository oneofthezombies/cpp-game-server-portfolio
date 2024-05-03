#ifndef KERO_ENGINE_PIN_DATA_H
#define KERO_ENGINE_PIN_DATA_H

#include <functional>

#include "kero/core/common.h"
#include "kero/core/result.h"

namespace kero {

template <typename T>
class PinDataFactory {
 public:
  explicit PinDataFactory() noexcept = default;
  virtual ~PinDataFactory() noexcept = default;
  CLASS_KIND_PINNABLE(PinDataFactory);

  virtual auto
  Create() noexcept -> Result<T*> = 0;
};

template <typename T>
using PinDataFactoryFn = std::function<Result<T*>()>;

}  // namespace kero

#endif  // KERO_ENGINE_PIN_DATA_H
