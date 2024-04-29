#ifndef KERO_ENGINE_CONFIG_COMPONENT_H
#define KERO_ENGINE_CONFIG_COMPONENT_H

#include "kero/engine/component.h"

namespace kero {

class ConfigComponent final : public Component {
 public:
  enum : Error::Code {
    kPortNotFound = 1,
    kPortParsingFailed,
    kUnknownArgument
  };

  virtual ~ConfigComponent() noexcept override = default;
  CLASS_KIND_MOVABLE(ConfigComponent);

  [[nodiscard]] virtual auto
  OnCreate(Agent& agent) noexcept -> Result<Void> override;

  virtual auto
  OnDestroy(Agent& agent) noexcept -> void override;

  virtual auto
  OnUpdate(Agent& agent) noexcept -> void override;

  [[nodiscard]] auto
  GetConfig() const noexcept -> const Dict&;

  [[nodiscard]] static auto
  FromArgs(int argc, char** argv) noexcept -> Result<ComponentPtr>;

 private:
  explicit ConfigComponent(Dict&& config) noexcept;

  Dict config_;
};

}  // namespace kero

#endif  // KERO_ENGINE_CONFIG_COMPONENT_H
