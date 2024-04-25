#include "engine.h"
#include "server/mail_center.h"

#include <cassert>

#if defined(__linux__)
#include "engine_linux.h"
using EngineImpl = EngineLinux;
using EngineImplBuilder = EngineLinuxBuilder;
#elif defined(_WIN32)
#error "Not implemented"
#elif defined(__APPLE__)
#error "Not implemented"
#else
#error "Unsupported platform"
#endif

auto CastEngineImpl(void *impl_raw) noexcept -> EngineImpl * {
  assert(impl_raw != nullptr && "impl must not be nullptr");
  return reinterpret_cast<EngineImpl *>(impl_raw);
}

auto EngineImplDeleter::operator()(void *impl_raw) const noexcept -> void {
  if (impl_raw == nullptr) {
    return;
  }

  delete CastEngineImpl(impl_raw);
}

Engine::Engine(EngineImplPtr &&impl) noexcept : impl_{std::move(impl)} {
  assert(impl_.get() != nullptr && "impl must not be nullptr");
}

auto Engine::Run() noexcept -> Result<Void> {
  return CastEngineImpl(impl_.get())->Run();
}

auto EngineBuilder::Build(const uint16_t port) const noexcept
    -> Result<Engine> {
  using ResultT = Result<Engine>;

  MailCenter::Global();
  auto result = EngineImplBuilder{}.Build(port);
  if (result.IsErr()) {
    return ResultT{std::move(result.Err())};
  }

  return ResultT{Engine{EngineImplPtr{new EngineImpl{std::move(result.Ok())}}}};
}
