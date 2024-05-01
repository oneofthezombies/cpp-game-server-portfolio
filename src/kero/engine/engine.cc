#include "engine.h"

#include "kero/engine/runner_builder.h"

using namespace kero;

auto
kero::Engine::CreateRunnerBuilder(std::string&& runner_name) -> RunnerBuilder {
  return RunnerBuilder{&engine_context_, std::move(runner_name)};
}
