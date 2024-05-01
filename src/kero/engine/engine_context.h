#ifndef KERO_ENGINE_ENGINE_CONTEXT_H
#define KERO_ENGINE_ENGINE_CONTEXT_H

#include "kero/engine/actor_system.h"
#include "kero/engine/pin_object_system.h"

namespace kero {

struct EngineContext {
  PinObjectSystem pin_object_system{};
  ActorSystem actor_system{};
};

}  // namespace kero

#endif  // KERO_ENGINE_ENGINE_CONTEXT_H