#include "engine.h"
#include "core.h"

#include <memory>

#if defined(__linux__)
#include "engine/linux.h"
using PlatformEngine = LinuxEngine;
using PlatformEngineFactory = LinuxEngineFactory;
#endif // defined(__linux__)

struct EngineImpl final : private core::NonCopyable, core::Movable {
  PlatformEngine platform_engine;

  EngineImpl(PlatformEngine &&platform_engine) noexcept
      : platform_engine{std::move(platform_engine)} {}
};

auto EngineFactory::Create() const noexcept -> Result<Engine> {
  auto result = PlatformEngineFactory{}.Create();
  if (result.IsErr()) {
    return Error{ServerErrorCode::kEngineCreationFailed};
  }

  return Engine{std::make_unique<EngineImpl>(std::move(result.Ok()))};
}
