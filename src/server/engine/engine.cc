#include "engine.h"

#include <cassert>

using namespace engine;

#if defined(__linux__)
#include "engine_linux.h"
using EngineImpl = EngineLinux;
#elif defined(_WIN32)
#error "Not implemented"
#elif defined(__APPLE__)
#error "Not implemented"
#else
#error "Unsupported platform"
#endif

namespace {

auto
CastEngineImpl(void *impl_raw) noexcept -> EngineImpl * {
  assert(impl_raw != nullptr && "impl must not be nullptr");
  return reinterpret_cast<EngineImpl *>(impl_raw);
}

}  // namespace

auto
engine::EngineImplRawDeleter::operator()(void *impl_raw) const noexcept
    -> void {
  if (impl_raw == nullptr) {
    return;
  }

  delete CastEngineImpl(impl_raw);
}

engine::Engine::Engine(EngineImplPtr &&impl) noexcept : impl_{std::move(impl)} {
  assert(impl_.get() != nullptr && "impl must not be nullptr");
}

auto
engine::Engine::AddEventLoop(std::string &&name,
                             EventLoopHandlerPtr &&handler) noexcept
    -> Result<Void> {
  return CastEngineImpl(impl_.get())
      ->AddEventLoop(std::move(name), std::move(handler));
}

auto
engine::Engine::Run() noexcept -> Result<Void> {
  return CastEngineImpl(impl_.get())->Run();
}

auto
engine::Engine::Builder::Build(Config &&config) const noexcept
    -> Result<Engine> {
  using ResultT = Result<Engine>;

  auto result = EngineImpl::Builder{}.Build(std::move(config));
  if (result.IsErr()) {
    return ResultT{Error::From(std::move(result.Err()))};
  }

  return ResultT{Engine{EngineImplPtr{new EngineImpl{std::move(result.Ok())}}}};
}
