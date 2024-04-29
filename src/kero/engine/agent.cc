#include "agent.h"

#include "kero/engine/component.h"
#include "kero/engine/constants.h"
#include "kero/engine/signal_component.h"

using namespace kero;

auto
kero::Agent::Run() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  auto res = CreateComponents();
  if (res.IsErr()) {
    return ResultT::Err(Error::From(res.TakeErr()));
  }

  auto signal = GetComponentAs<SignalComponent>(ComponentKind::kSignal);
  if (!signal) {
    // TODO: log warning
  }

  auto is_interrupted = false;
  while (true) {
    if (signal) {
      is_interrupted = signal.Unwrap().IsInterrupted();
    }

    UpdateComponents();
  }

  DestroyComponents();

  if (is_interrupted) {
    return ResultT::Err(Error::From(kInterrupted));
  }

  return ResultT::Ok(Void{});
}

auto
kero::Agent::Dispatch(const std::string& event, const Dict& data) noexcept
    -> void {
  auto it = events_.find(event);
  if (it == events_.end()) {
    return;
  }

  for (const auto component_kind : it->second) {
    auto component = GetComponent(component_kind);
    if (component.IsNone()) {
      continue;
    }

    component.Unwrap().OnEvent(*this, event, data);
  }
}

auto
kero::Agent::SubscribeEvent(const std::string& event,
                            const Component::Kind component_kind) noexcept
    -> bool {
  auto it = events_.find(event);
  if (it == events_.end()) {
    it = events_.try_emplace(event).first;
  }

  auto& component_kinds = it->second;
  if (component_kinds.contains(component_kind)) {
    return false;
  }

  component_kinds.insert(component_kind);
  return true;
}

auto
kero::Agent::UnsubscribeEvent(const std::string& event,
                              const Component::Kind component_kind) noexcept
    -> bool {
  auto it = events_.find(event);
  if (it == events_.end()) {
    return false;
  }

  auto& component_kinds = it->second;
  if (!component_kinds.contains(component_kind)) {
    return false;
  }

  component_kinds.erase(component_kind);
  return true;
}

auto
kero::Agent::GetComponent(const Component::Kind component_kind) const noexcept
    -> OptionRef<Component&> {
  auto it = components_.find(component_kind);
  if (it == components_.end()) {
    return None;
  }

  return OptionRef<Component&>{*it->second};
}

auto
kero::Agent::HasComponent(const Component::Kind component_kind) const noexcept
    -> bool {
  return components_.contains(component_kind);
}

auto
kero::Agent::CreateComponents() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  for (auto& [_, component] : components_) {
    auto res = component->OnCreate(*this);
    if (res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }
  }

  return ResultT::Ok(Void{});
}

auto
kero::Agent::DestroyComponents() noexcept -> void {
  for (auto& [_, component] : components_) {
    component->OnDestroy(*this);
  }
}

auto
kero::Agent::UpdateComponents() noexcept -> void {
  for (auto& [_, component] : components_) {
    component->OnUpdate(*this);
  }
}

kero::ThreadAgent::~ThreadAgent() noexcept {
  if (IsRunning()) {
    [[maybe_unused]] auto stopped = Stop();
  }
}

auto
kero::ThreadAgent::Start(Agent&& agent) noexcept -> bool {
  if (IsRunning()) {
    return false;
  }

  thread_ = std::thread{ThreadAgent::ThreadMain, std::move(agent)};
  return true;
}

auto
kero::ThreadAgent::Stop() noexcept -> bool {
  if (!IsRunning()) {
    return false;
  }

  thread_.join();
  return true;
}

auto
kero::ThreadAgent::IsRunning() const noexcept -> bool {
  return thread_.joinable();
}

auto
kero::ThreadAgent::ThreadMain(Agent&& agent) -> void {
  if (auto res = agent.Run(); res.IsErr()) {
    // TODO: log error
  }
}
