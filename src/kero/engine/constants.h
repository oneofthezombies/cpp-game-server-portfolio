#ifndef KERO_ENGINE_CONSTANTS_H
#define KERO_ENGINE_CONSTANTS_H

#include "kero/core/error.h"

namespace kero {

enum ErrorCode : Error::Code {
  kInterrupted = 3,
};

struct EventShutdown {
  static constexpr auto kEvent = "shutdown";
};

}  // namespace kero

#endif  // KERO_ENGINE_CONSTANTS_H
