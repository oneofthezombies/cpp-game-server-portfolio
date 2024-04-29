#include "center.h"

#include "global_context.h"

using namespace kero;

auto
kero::Center::UseSystemErrorStream(std::ostream& stream) noexcept -> void {
  GetGlobalContext().UseSystemErrorStream(stream);
}

auto
kero::Center::Shutdown(ShutdownConfig&& config) noexcept -> void {
  GetGlobalContext().Shutdown(std::move(config));
}

auto
kero::Center::AddTransport(std::unique_ptr<Transport>&& transport) noexcept
    -> void {
  GetGlobalContext().AddTransport(std::move(transport));
}
