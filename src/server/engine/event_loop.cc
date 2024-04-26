#include "event_loop.h"

#include <cassert>

#if defined(__linux__)
#include "event_loop_linux.h"
using EventLoopImplRaw = EventLoopLinux;
#elif defined(_WIN32)
#error "Not implemented"
#elif defined(__APPLE__)
#error "Not implemented"
#else
#error "Unsupported platform"
#endif

using namespace engine;

auto CastEventLoopImplRaw(void *impl_raw) noexcept -> engine::EventLoopImpl * {
  return static_cast<engine::EventLoopImpl *>(impl_raw);
}

auto engine::EventLoopImplRawDeleter::operator()(void *impl_raw) const noexcept
    -> void {
  if (impl_raw == nullptr) {
    return;
  }

  delete CastEventLoopImplRaw(impl_raw);
}

engine::EventLoop::EventLoop(EventLoopImpl &&impl) noexcept
    : impl_{std::move(impl)} {
  assert(impl_.get() != nullptr && "impl must not be nullptr");
}

auto engine::EventLoop::Run() noexcept -> Result<core::Void> {
  return CastEventLoopImpl(impl_.get())->Run();
}

auto EventLoop::Builder::Build() const noexcept -> Result<EventLoop> {
  using ResultT = Result<EventLoop>;

  auto result = EventLoopImpl::Builder{}.Build();
  if (result.IsErr()) {
    return ResultT{std::move(result.Err())};
  }

  return ResultT{
      EventLoop{EventLoopImpl{new EventLoopImplRaw{std::move(result.Ok())}}}};
}
