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

  kServiceKindId_Service,
  kServiceKindId_Actor,
  kServiceKindId_Signal,

  kServiceKindId_EngineEnd,
};

struct EventShutdown {
  static constexpr auto kEvent = "shutdown";
};

}  // namespace kero

#endif  // KERO_ENGINE_COMMON_H
