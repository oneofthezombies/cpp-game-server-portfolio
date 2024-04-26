#include "event_loop.h"

#include <cassert>

#if defined(__linux__)
#include "event_loop_linux.h"
using EventLoopImpl = EventLoopLinux;
#elif defined(_WIN32)
#error "Not implemented"
#elif defined(__APPLE__)
#error "Not implemented"
#else
#error "Unsupported platform"
#endif

auto CastEventLoopImplRaw(void *impl_raw) noexcept -> EventLoopImpl * {
  return static_cast<EventLoopImpl *>(impl_raw);
}

auto EventLoopImplDeleter::operator()(void *impl_raw) const noexcept -> void {
  if (impl_raw == nullptr) {
    return;
  }

  delete CastEventLoopImplRaw(impl_raw);
}

EventLoop::EventLoop(EventLoopImplPtr &&impl) noexcept
    : impl_{std::move(impl)} {
  assert(impl_.get() != nullptr && "impl must not be nullptr");
}

auto EventLoop::Run() noexcept -> Result<Void> {
  return CastEventLoopImpl(impl_.get())->Run();
}

auto EventLoop::Builder::Build() const noexcept -> Result<EventLoop> {
  using ResultT = Result<EventLoop>;

  auto result = EventLoopImpl::Builder{}.Build();
  if (result.IsErr()) {
    return ResultT{std::move(result.Err())};
  }

  return ResultT{
      EventLoop{EventLoopImplPtr{new EventLoopImpl{std::move(result.Ok())}}}};
}
