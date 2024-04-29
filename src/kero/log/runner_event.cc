#include "runner_event.h"

using namespace kero;

kero::runner_event::Shutdown::Shutdown(ShutdownConfig&& config) noexcept
    : config{std::move(config)} {}
