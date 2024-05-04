#include "service.h"

#include "kero/core/utils.h"
#include "kero/engine/runner_context.h"

using namespace kero;

kero::Service::Service(
    const Pin<RunnerContext> runner_context,
    DependencyDeclarations&& dependency_declarations) noexcept
    : runner_context_{runner_context},
      dependency_declarations_{std::move(dependency_declarations)} {}

auto
kero::Service::GetKindId() const noexcept -> ServiceKindId {
  return Service::GetKindId();
}

auto
kero::Service::GetKindName() const noexcept -> ServiceKindName {
  return Service::GetKindName();
}

auto
kero::Service::GetDependencyDeclarations() const noexcept
    -> const DependencyDeclarations& {
  return dependency_declarations_;
}

auto
kero::Service::GetDependency(const ServiceKindId service_kind_id) noexcept
    -> Borrow<Service> {
  return dependency_map_.GetService(service_kind_id);
}

auto
kero::Service::GetDependency(const ServiceKindName service_kind_name) noexcept
    -> Borrow<Service> {
  return dependency_map_.GetService(service_kind_name);
}

auto
kero::Service::SubscribeEvent(const std::string& event) -> Result<Void> {
  return runner_context_->SubscribeEvent(event, GetKindId());
}

auto
kero::Service::UnsubscribeEvent(const std::string& event) -> Result<Void> {
  return runner_context_->UnsubscribeEvent(event, GetKindId());
}

auto
kero::Service::InvokeEvent(const std::string& event,
                           const FlatJson& data) noexcept -> Result<Void> {
  return runner_context_->InvokeEvent(event, data);
}

auto
kero::Service::OnCreate() noexcept -> Result<Void> {
  return OkVoid();
}

auto
kero::Service::OnDestroy() noexcept -> void {
  // noop
}

auto
kero::Service::OnUpdate() noexcept -> void {
  // noop
}

auto
kero::Service::OnEvent(const std::string& event,
                       const FlatJson& data) noexcept -> void {
  // noop
}
