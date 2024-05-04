#ifndef KERO_ENGINE_ENGINE_CONTEXT_H
#define KERO_ENGINE_ENGINE_CONTEXT_H

#include "kero/engine/actor_system.h"

namespace kero {

struct EngineContext {
  Own<ThreadActorSystem> thread_actor_system;

  explicit EngineContext() noexcept;
};

}  // namespace kero

#endif  // KERO_ENGINE_ENGINE_CONTEXT_H
