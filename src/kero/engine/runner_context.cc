#include "runner_context.h"

#include "kero/engine/runner.h"

using namespace kero;

kero::RunnerContext::RunnerContext(const Pinned<Runner> runner) noexcept
    : runner_{runner} {}

auto
kero::RunnerContext::GetService(const ServiceKind::Id service_kind_id)
    const noexcept -> OptionRef<Service&> {
  return runner_.Unwrap().GetService(service_kind_id);
}

auto
kero::RunnerContext::GetService(const ServiceKind::Name& service_kind_name)
    const noexcept -> OptionRef<Service&> {
  return runner_.Unwrap().GetService(service_kind_name);
}

auto
kero::RunnerContext::HasService(
    const ServiceKind::Id service_kind_id) const noexcept -> bool {
  return runner_.Unwrap().HasService(service_kind_id);
}

auto
kero::RunnerContext::HasService(
    const ServiceKind::Name& service_kind_name) const noexcept -> bool {
  return runner_.Unwrap().HasService(service_kind_name);
}

auto
kero::RunnerContext::HasServiceIs(
    const ServiceKind::Id service_kind_id) const noexcept -> bool {
  return runner_.Unwrap().HasServiceIs(service_kind_id);
}

auto
kero::RunnerContext::HasServiceIs(
    const ServiceKind::Name& service_kind_name) const noexcept -> bool {
  return runner_.Unwrap().HasServiceIs(service_kind_name);
}

auto
kero::RunnerContext::SubscribeEvent(const std::string& event,
                                    const ServiceKind& kind) -> Result<Void> {
  return runner_.Unwrap().SubscribeEvent(event, kind);
}

auto
kero::RunnerContext::UnsubscribeEvent(const std::string& event,
                                      const ServiceKind& kind) -> Result<Void> {
  return runner_.Unwrap().UnsubscribeEvent(event, kind);
}

auto
kero::RunnerContext::InvokeEvent(const std::string& event,
                                 const Dict& data) noexcept -> Result<Void> {
  return runner_.Unwrap().InvokeEvent(event, data);
}
