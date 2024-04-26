#include "engine.h"

#include <cassert>

#if defined(__linux__)
#include "engine_linux.h"
using EngineImplRaw = EngineLinux;
#elif defined(_WIN32)
#error "Not implemented"
#elif defined(__APPLE__)
#error "Not implemented"
#else
#error "Unsupported platform"
#endif

using namespace engine;

namespace {

auto CastEngineImplRaw(void *impl_raw) noexcept -> EngineImplRaw * {
  assert(impl_raw != nullptr && "impl must not be nullptr");
  return reinterpret_cast<EngineImplRaw *>(impl_raw);
}

} // namespace

auto engine::EngineImplRawDeleter::operator()(void *impl_raw) const noexcept
    -> void {
  if (impl_raw == nullptr) {
    return;
  }

  delete CastEngineImplRaw(impl_raw);
}

engine::Engine::Engine(EngineImpl &&impl) noexcept : impl_{std::move(impl)} {
  assert(impl_.get() != nullptr && "impl must not be nullptr");
}

auto engine::Engine::Run() noexcept -> Result<Void> {
  return CastEngineImplRaw(impl_.get())->Run();
}

auto engine::Engine::Builder::Build(const Options &options) const noexcept
    -> Result<Engine> {
  using ResultT = Result<Engine>;

  auto result = EngineImplRaw::Builder{}.Build(options);
  if (result.IsErr()) {
    return ResultT{std::move(result.Err())};
  }

  return ResultT{Engine{EngineImpl{new EngineImplRaw{std::move(result.Ok())}}}};
}
