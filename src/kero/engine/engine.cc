#include "engine.h"

#include "component.h"

using namespace kero;

auto
kero::Engine::Run() noexcept -> void {
  while (true) {
    Update();
  }
}

auto
kero::Engine::Dispatch(const std::string& event, const Dict& data) noexcept
    -> void {
  auto it = events_.find(event);
  if (it == events_.end()) {
    return;
  }

  for (const auto& component_name : it->second) {
    auto component = GetComponent(component_name);
    if (component.IsNone()) {
      continue;
    }

    component.Unwrap().OnEvent(*this, event, data);
  }
}

auto
kero::Engine::SubscribeEvent(const std::string& event,
                             const std::string& component_name) noexcept
    -> bool {
  auto it = events_.find(event);
  if (it == events_.end()) {
    it = events_.try_emplace(event).first;
  }

  auto& components = it->second;
  if (components.contains(component_name)) {
    return false;
  }

  components.insert(component_name);
  return true;
}

auto
kero::Engine::UnsubscribeEvent(const std::string& event,
                               const std::string& component_name) noexcept
    -> bool {
  auto it = events_.find(event);
  if (it == events_.end()) {
    return false;
  }

  auto& components = it->second;
  if (!components.contains(component_name)) {
    return false;
  }

  components.erase(component_name);
  return true;
}

auto
kero::Engine::GetComponent(const std::string& name) noexcept
    -> OptionRef<Component&> {
  auto it = components_.find(name);
  if (it == components_.end()) {
    return None;
  }

  return OptionRef<Component&>{*it->second};
}

auto
kero::Engine::Update() noexcept -> void {
  for (auto& [_, component] : components_) {
    component->OnUpdate(*this);
  }
}

kero::ThreadEngine::~ThreadEngine() noexcept {
  if (IsRunning()) {
    [[maybe_unused]] auto stopped = Stop();
  }
}

auto
kero::ThreadEngine::Start(Engine&& engine) noexcept -> bool {
  if (IsRunning()) {
    return false;
  }

  thread_ = std::thread{ThreadMain, std::move(engine)};
  return true;
}

auto
kero::ThreadEngine::Stop() noexcept -> bool {
  if (!IsRunning()) {
    return false;
  }

  thread_.join();
  return true;
}

auto
kero::ThreadEngine::IsRunning() const noexcept -> bool {
  return thread_.joinable();
}

auto
kero::ThreadEngine::ThreadMain(Engine&& engine) -> void {
  engine.Run();
}
