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
  Dispatch(const std::string& event, const Dict& data) noexcept -> void;

  template <typename T>
    requires std::is_base_of_v<Component, T>
  [[nodiscard]] auto
  AddComponent(std::unique_ptr<T>&& component) noexcept -> bool {
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
  explicit ThreadEngine() noexcept = default;
  ~ThreadEngine() noexcept;

  [[nodiscard]] auto
  Start(Engine&& engine) noexcept -> bool;

  [[nodiscard]] auto
  Stop() noexcept -> bool;

  [[nodiscard]] auto
  IsRunning() const noexcept -> bool;

 private:
  static auto
  ThreadMain(Engine&& engine) -> void;

  std::thread thread_;
};

}  // namespace kero

#endif  // KERO_ENGINE_ENGINE_H
