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
kero::Service::GetRunnerContext() noexcept -> RunnerContext& {
  return *runner_context_;
}

auto
kero::Service::GetDependency(const ServiceKind& kind) noexcept
    -> Borrow<Service> {
  return GetDependency(kind.id);
}

auto
kero::Service::GetDependency(const ServiceKind::Id kind_id) noexcept
    -> Borrow<Service> {
  return dependency_map_.GetService(kind_id);
}

auto
kero::Service::GetDependency(const ServiceKind::Name& kind_name) noexcept
    -> Borrow<Service> {
  return dependency_map_.GetService(kind_name);
}

auto
kero::Service::Is(const ServiceKind& kind) const noexcept -> bool {
  return Is(kind.id);
}

auto
kero::Service::Is(const ServiceKind::Id kind_id) const noexcept -> bool {
  return kind_.id == kind_id;
}

auto
kero::Service::Is(const ServiceKind::Name& kind_name) const noexcept -> bool {
  return kind_.name == kind_name;
}

auto
kero::Service::SubscribeEvent(const std::string& event) -> Result<Void> {
  return runner_context_->SubscribeEvent(event, kind_);
}

auto
kero::Service::UnsubscribeEvent(const std::string& event) -> Result<Void> {
  return runner_context_->UnsubscribeEvent(event, kind_);
}

auto
kero::Service::InvokeEvent(const std::string& event, const Json& data) noexcept
    -> Result<Void> {
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
kero::Service::OnEvent(const std::string& event, const Json& data) noexcept
    -> void {
  // noop
}
