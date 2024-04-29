#ifndef KERO_ENGINE_AGENT_H
#define KERO_ENGINE_AGENT_H

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "kero/core/dict.h"
#include "kero/engine/component.h"

namespace kero {

class Agent final {
 public:
  enum : Error::Code {
    kInterrupted = 1,
  };

  ~Agent() noexcept = default;
  CLASS_KIND_MOVABLE(Agent);

  [[nodiscard]] auto
  Run() noexcept -> Result<Void>;

  auto
  Dispatch(const std::string& event, const Dict& data) noexcept -> void;

  template <typename T>
    requires std::is_base_of_v<Component, T>
  [[nodiscard]] auto
  AddComponent(std::unique_ptr<T>&& component) noexcept -> bool {
    auto [it, inserted] =
        components_.try_emplace(component->GetKind(), std::move(component));
    return inserted;
  }

  [[nodiscard]] auto
  SubscribeEvent(const std::string& event,
                 const Component::Kind component_kind) noexcept -> bool;

  [[nodiscard]] auto
  UnsubscribeEvent(const std::string& event,
                   const Component::Kind component_kind) noexcept -> bool;

  [[nodiscard]] auto
  GetComponent(const Component::Kind component_kind) const noexcept
      -> OptionRef<Component&>;

  template <typename T>
    requires std::is_base_of_v<Component, T>
  [[nodiscard]] auto
  GetComponentAs(const Component::Kind component_kind) const noexcept
      -> OptionRef<T&> {
    auto component = GetComponent(component_kind);
    if (component.IsNone()) {
      return None;
    }

    return component.Unwrap().As<T>(component_kind);
  }

  [[nodiscard]] auto
  HasComponent(const Component::Kind component_kind) const noexcept -> bool;

  template <typename T>
    requires std::is_base_of_v<Component, T>
  [[nodiscard]] auto
  HasComponentIs(const Component::Kind component_kind) const noexcept -> bool {
    auto component = GetComponent(component_kind);
    if (component.IsNone()) {
      return false;
    }

    return component.Unwrap().Is<T>(component_kind);
  }

 private:
  auto
  CreateComponents() noexcept -> Result<Void>;

  auto
  DestroyComponents() noexcept -> void;

  auto
  UpdateComponents() noexcept -> void;

  std::unordered_map<Component::Kind, ComponentPtr> components_;
  std::unordered_map<std::string /* event */,
                     std::unordered_set<Component::Kind>>
      events_;
};

class ThreadAgent final {
 public:
  explicit ThreadAgent() noexcept = default;
  ~ThreadAgent() noexcept;

  [[nodiscard]] auto
  Start(Agent&& agent) noexcept -> bool;

  [[nodiscard]] auto
  Stop() noexcept -> bool;

  [[nodiscard]] auto
  IsRunning() const noexcept -> bool;

 private:
  static auto
  ThreadMain(Agent&& agent) -> void;

  std::thread thread_;
};

}  // namespace kero

#endif  // KERO_ENGINE_AGENT_H
