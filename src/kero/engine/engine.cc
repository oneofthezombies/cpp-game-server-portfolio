#include "engine.h"

#include "kero/engine/runner_builder.h"

using namespace kero;

auto
kero::Engine::Global() -> Engine& {
  static std::unique_ptr<Engine> engine{};
  static std::once_flag flag{};
  std::call_once(flag, []() { engine = std::make_unique<Engine>(); });
  return *engine;
}

auto
kero::Engine::CreateRunnerBuilder(std::string&& name) -> Result<RunnerBuilder> {
  using ResultT = Result<RunnerBuilder>;
  return ResultT::Ok(
      std::move(RunnerBuilder{&engine_context_, std::move(name)}));
}
