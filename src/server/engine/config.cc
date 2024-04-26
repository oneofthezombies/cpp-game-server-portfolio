#include "config.h"

using namespace engine;

auto engine::Config::Validate() const noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (port == kUndefinedPort) {
    return ResultT{Error{Symbol::kConfigPortUndefined}};
  }

  return ResultT{Void{}};
}
