#include "engine.h"

#include <cassert>

#if defined(__linux__)
#include "engine_linux.h"
using PlatformEngine = LinuxEngine;
using PlatformEngineBuilder = LinuxEngineBuilder;
#elif defined(_WIN32)
#error "Not implemented"
#include "engine_windows.h"
using PlatformEngine = WindowsEngine;
using PlatformEngineBuilder = WindowsEngineBuilder;
#elif defined(__APPLE__)
#error "Not implemented"
#include "engine_macos.h"
using PlatformEngine = MacOsEngine;
using PlatformEngineBuilder = MacOsEngineBuilder;
#else
#error "Unsupported platform"
#endif

auto CastPlatformEngine(void *impl) noexcept -> PlatformEngine * {
  return static_cast<PlatformEngine *>(impl);
}

auto DeletePlatformEngine(void *&impl) noexcept -> void {
  if (impl == nullptr) {
    return;
  }

  delete CastPlatformEngine(impl);
  impl = nullptr;
}

Engine::Engine(void *impl) noexcept : impl_{impl} {
  assert(impl_ != nullptr && "Impl must not be null");
}

Engine::Engine(Engine &&other) noexcept : impl_{other.impl_} {
  other.impl_ = nullptr;
}

Engine::~Engine() noexcept { DeletePlatformEngine(impl_); }

auto Engine::operator=(Engine &&other) noexcept -> Engine & {
  if (this == &other) {
    return *this;
  }

  DeletePlatformEngine(impl_);
  impl_ = other.impl_;
  other.impl_ = nullptr;

  return *this;
}

auto Engine::Run() noexcept -> Result<core::Void> {
  assert(impl_ != nullptr && "Impl must not be null");

  const auto engine = CastPlatformEngine(impl_);
  return engine->Run();
}

auto EngineBuilder::Build(const uint16_t port) const noexcept
    -> Result<Engine> {
  auto result = PlatformEngineBuilder{}.Build(port);
  if (result.IsErr()) {
    return Error{std::move(result.Err())};
  }

  return Engine{new PlatformEngine{std::move(result.Ok())}};
}
