#include "main_event_loop_handler_builder.h"

using namespace engine;

#if defined(__linux__)
#include "main_event_loop_handler_linux.h"
using MainEventLoopHandler = MainEventLoopHandlerLinux;
#elif defined(_WIN32)
#error "Not implemented"
#elif defined(__APPLE__)
#error "Not implemented"
#else
#error "Unsupported platform"
#endif

engine::MainEventLoopHandlerBuilder::MainEventLoopHandlerBuilder(
    std::string &&primary_event_loop_name) noexcept
    : primary_event_loop_name_(std::move(primary_event_loop_name)) {}

auto engine::MainEventLoopHandlerBuilder::Build() noexcept
    -> Result<EventLoopHandlerPtr> {
  using ResultT = Result<EventLoopHandlerPtr>;

  if (primary_event_loop_name_.empty()) {
    return ResultT{
        Error{Symbol::MainEventLoopHandlerBuilderPrimaryEventLoopNameEmpty}};
  }

  return ResultT{EventLoopHandlerPtr{
      new MainEventLoopHandler{std::move(primary_event_loop_name_)}}};
}
