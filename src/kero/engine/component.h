#ifndef KERO_ENGINE_COMPONENT_H
#define KERO_ENGINE_COMPONENT_H

#include <string>

#include "kero/core/common.h"
#include "kero/core/dict.h"

namespace kero {

class Engine;

class Component {
 public:
  explicit Component(std::string&& name) noexcept;
  virtual ~Component() noexcept = default;
  CLASS_KIND_MOVABLE(Component);

  [[nodiscard]] auto
  GetName() const noexcept -> std::string_view;

  template <typename T>
    requires std::is_base_of_v<Component, T>
  [[nodiscard]] auto
  As(const std::string& name) noexcept -> OptionRef<T&> {
    if (name != name_) {
      return None;
    }

    return static_cast<T&>(*this);
  }

  virtual auto
  OnCreate(Engine& engine) noexcept -> void {
    /* noop */
  }

  virtual auto
  OnUpdate(Engine& engine) noexcept -> void {
    /* noop */
  }

  virtual auto
  OnEvent(Engine& engine, const std::string& event, const Dict& data) noexcept
      -> void {
    /* noop */
  }

 private:
  std::string name_;
};

}  // namespace kero

#endif  // KERO_ENGINE_COMPONENT_H
