#ifndef KERO_ENGINE_CONSTANTS_H
#define KERO_ENGINE_CONSTANTS_H

#include "kero/core/error.h"
#include "kero/engine/service_kind.h"

namespace kero {

enum ErrorCode : Error::Code {
  kInterrupted = 3,
};

enum EngineServiceKindId : ServiceKindId {
  kServiceKindIdActor = 1,
  kServiceKindIdSignal = 2,
};

struct EventShutdown {
  static constexpr auto kEvent = "shutdown";
};

}  // namespace kero

#endif  // KERO_ENGINE_CONSTANTS_H
