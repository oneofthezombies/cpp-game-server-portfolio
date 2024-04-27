#include "config.h"

using namespace engine;

auto engine::Config::Validate() const noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (port == kUndefinedPort) {
    return ResultT{Error{Symbol::kConfigPortUndefined}};
  }

  if (primary_event_loop_name.empty()) {
    return ResultT{Error{Symbol::kConfigPrimaryEventLoopNameEmpty}};
  }

  return ResultT{Void{}};
}
