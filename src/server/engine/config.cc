#include "config.h"

using namespace engine;

auto
engine::Config::Validate() const noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (port == kUndefinedPort) {
    return ResultT{Error::From(kConfigPortUndefined)};
  }

  if (primary_event_loop_name.empty()) {
    return ResultT{Error::From(kConfigPrimaryEventLoopNameEmpty)};
  }

  return ResultT{Void{}};
}
