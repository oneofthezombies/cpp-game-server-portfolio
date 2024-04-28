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
kero::Engine::Update() noexcept -> void {
  for (auto& [_, component] : components_) {
    component->OnUpdate(*this);
  }
}

auto
kero::Engine::AddEvent(const std::string& event,
                       const std::string& component_name) noexcept -> bool {
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
kero::Engine::RemoveEvent(const std::string& event,
                          const std::string& component_name) noexcept -> bool {
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
kero::ThreadEngine::Start(Engine&& runner) -> bool {
  if (thread_.joinable()) {
    return false;
  }

  thread_ = std::thread{&ThreadMain, std::move(runner)};
  return true;
}

auto
kero::ThreadEngine::Stop() -> bool {
  if (!thread_.joinable()) {
    return false;
  }

  thread_.join();
  return true;
}

auto
kero::ThreadEngine::ThreadMain(Engine&& runner) -> void {
  runner.Run();
}
