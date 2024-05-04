#ifndef KERO_ENGINE_ENGINE_CONTEXT_H
#define KERO_ENGINE_ENGINE_CONTEXT_H

#include "kero/engine/actor_system.h"
#include "kero/engine/pin_system.h"

namespace kero {

struct EngineContext {
  Own<PinSystem> pin_system;
  Own<ActorSystem> actor_system;
  Own<ThreadActorSystem> thread_actor_system;
};

}  // namespace kero

#endif  // KERO_ENGINE_ENGINE_CONTEXT_H
