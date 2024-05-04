#include "engine_context.h"

#include "actor_system.h"

using namespace kero;

kero::EngineContext::EngineContext() noexcept
    : thread_actor_system{std::make_unique<ThreadActorSystem>(
          std::make_unique<ActorSystem>())} {}
