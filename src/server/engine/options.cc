#include "options.h"

using namespace engine;

auto engine::Options::Validate() const noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (port == kUndefinedPort) {
    return ResultT{Error{Symbol::kOptionsPortUndefined}};
  }

  return ResultT{Void{}};
}
