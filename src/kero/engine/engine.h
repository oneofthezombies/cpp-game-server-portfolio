#ifndef KERO_ENGINE_ENGINE_H
#define KERO_ENGINE_ENGINE_H

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "kero/core/dict.h"

namespace kero {

class Component;

class Engine final {
 public:
  ~Engine() noexcept = default;

  auto
  Run() noexcept -> void;

  auto
  Dispatch(std::string&& event, Dict&& data) noexcept -> void;

  template <typename T>
    requires std::is_base_of_v<Component, T>
  [[nodiscard]] auto
  AddComponent() noexcept -> bool {
    auto component = std::make_unique<T>();
    auto [it, inserted] =
        components_.try_emplace(component->GetName(), std::move(component));
    if (!inserted) {
      return false;
    }

    it->second->OnCreate(*this);
  }

  [[nodiscard]] auto
  AddEvent(const std::string& event, const std::string& component_name) noexcept
      -> bool;

  [[nodiscard]] auto
  RemoveEvent(const std::string& event,
              const std::string& component_name) noexcept -> bool;

  [[nodiscard]] auto
  GetComponent(const std::string& name) noexcept -> OptionRef<Component&>;

 private:
  auto
  Update() noexcept -> void;

  std::unordered_map<std::string /* component name */,
                     std::unique_ptr<Component>>
      components_;
  std::unordered_map<std::string /* event */,
                     std::unordered_set<std::string /* component name */>>
      events_;
};

class ThreadEngine final {
 public:
  auto
  Start(Engine&& runner) -> bool;

  auto
  Stop() -> bool;

 private:
  static auto
  ThreadMain(Engine&& runner) -> void;

  std::thread thread_;
};

}  // namespace kero

#endif  // KERO_ENGINE_ENGINE_H
