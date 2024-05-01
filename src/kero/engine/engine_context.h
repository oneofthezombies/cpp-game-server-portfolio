#ifndef KERO_ENGINE_ENGINE_CONTEXT_H
#define KERO_ENGINE_ENGINE_CONTEXT_H

#include "kero/engine/actor_system.h"
#include "kero/engine/pinning_system.h"

namespace kero {

struct EngineContext {
  PinningSystem pinning_system{};
  ActorSystem actor_system{};
};

}  // namespace kero

#endif  // KERO_ENGINE_ENGINE_CONTEXT_H
