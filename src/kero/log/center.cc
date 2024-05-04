#include "center.h"

#include "global_context.h"

using namespace kero;

auto
kero::Center::UseStreamForLoggingSystemError(std::ostream& stream) noexcept
    -> void {
  GetGlobalContext().UseStreamForLoggingSystemError(stream);
}

auto
kero::Center::Shutdown(ShutdownConfig&& config) noexcept -> void {
  GetGlobalContext().Shutdown(std::move(config));
}

auto
kero::Center::AddTransport(Own<Transport>&& transport) noexcept -> void {
  GetGlobalContext().AddTransport(std::move(transport));
}
