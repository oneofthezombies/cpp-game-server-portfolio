#include "config.h"

using namespace engine;

auto engine::Config::Validate() const noexcept -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  if (port == kUndefinedPort) {
    return ResultT{Error{Symbol::kConfigPortUndefined}};
  }

  if (root_service == nullptr) {
    return ResultT{Error{Symbol::kConfigRootServiceNotFound}};
  }

  return ResultT{core::Void{}};
}
