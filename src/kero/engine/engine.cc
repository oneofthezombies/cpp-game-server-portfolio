#include "engine.h"

using namespace kero;

auto
kero::Engine::Global() -> Engine& {
  static std::unique_ptr<Engine> engine{};
  static std::once_flag flag{};
  std::call_once(flag, []() { engine = std::make_unique<Engine>(); });
  return *engine;
}
