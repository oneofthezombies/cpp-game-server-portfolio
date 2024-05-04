#ifndef KERO_ENGINE_COMMON_H
#define KERO_ENGINE_COMMON_H

#include "kero/core/error.h"
#include "kero/engine/service_kind.h"

namespace kero {

enum ErrorCode : Error::Code {
  kInterrupted = 3,
};

enum : ServiceKindId {
  kServiceKindId_EngineBegin = 0,

  kServiceKindId_Actor,
  kServiceKindId_Signal,

  kServiceKindId_EngineEnd,
};

struct EventShutdown {
  static constexpr auto kEvent = "shutdown";
};

}  // namespace kero

#define KERO_SERVICE_KIND(kind_id, kind_name)             \
  static constexpr ServiceKindId kKindId = kind_id;       \
  static constexpr ServiceKindName kKindName = kind_name; \
  [[nodiscard]] inline virtual auto GetKindId()           \
      const noexcept -> ServiceKindId override {          \
    return kKindId;                                       \
  }                                                       \
  [[nodiscard]] inline virtual auto GetKindName()         \
      const noexcept -> ServiceKindName override {        \
    return kKindName;                                     \
  }

#endif  // KERO_ENGINE_COMMON_H
