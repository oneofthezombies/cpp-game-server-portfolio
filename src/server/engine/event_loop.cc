#include "event_loop.h"
#include "server/engine/event_loop_handler.h"

#include <cassert>

using namespace engine;

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

auto CastEventLoopImpl(void *impl_raw) noexcept -> EventLoopImpl * {
  return static_cast<EventLoopImpl *>(impl_raw);
}

auto engine::EventLoopImplRawDeleter::operator()(void *impl_raw) const noexcept
    -> void {
  if (impl_raw == nullptr) {
    return;
  }

  delete CastEventLoopImpl(impl_raw);
}

engine::EventLoop::EventLoop(EventLoopImplPtr &&impl) noexcept
    : impl_{std::move(impl)} {
  assert(impl_.get() != nullptr && "impl must not be nullptr");
}

auto engine::EventLoop::Run() noexcept -> Result<Void> {
  return CastEventLoopImpl(impl_.get())->Run();
}

auto EventLoop::Builder::Build(std::string &&name,
                               EventLoopHandlerPtr &&handler) const noexcept
    -> Result<EventLoop> {
  using ResultT = Result<EventLoop>;

  auto result =
      EventLoopImpl::Builder{}.Build(std::move(name), std::move(handler));
  if (result.IsErr()) {
    return ResultT{std::move(result.Err())};
  }

  return ResultT{
      EventLoop{EventLoopImplPtr{new EventLoopImpl{std::move(result.Ok())}}}};
}
