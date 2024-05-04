#include "engine.h"

#include "kero/core/borrow.h"
#include "kero/engine/runner_builder.h"

using namespace kero;

kero::Engine::Engine() noexcept : engine_context_{} {
  engine_context_->pin_system = std::make_unique<PinSystem>();
  engine_context_->actor_system = std::make_unique<ActorSystem>();
  engine_context_->thread_actor_system = std::make_unique<ThreadActorSystem>(
      Borrow{engine_context_->actor_system});
}

auto
kero::Engine::CreateRunnerBuilder(std::string&& runner_name) -> RunnerBuilder {
  return RunnerBuilder{Borrow{engine_context_}, std::move(runner_name)};
}

auto
kero::Engine::Start() -> Result<Void> {
  using ResultT = Result<Void>;

  auto res = engine_context_->thread_actor_system->Start();
  if (res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  return OkVoid();
}

auto
kero::Engine::Stop() -> Result<Void> {
  using ResultT = Result<Void>;

  auto res = engine_context_->thread_actor_system->Stop();
  if (res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  return OkVoid();
}
