#ifndef KERO_ENGINE_CONFIG_SERVICE_H
#define KERO_ENGINE_CONFIG_SERVICE_H

#include "kero/engine/service.h"

namespace kero {

class ConfigService final : public Service {
 public:
  enum : Error::Code {
    kPortNotFound = 1,
    kPortParsingFailed,
    kUnknownArgument
  };

  virtual ~ConfigService() noexcept override = default;
  CLASS_KIND_MOVABLE(ConfigService);

  [[nodiscard]] virtual auto
  OnCreate(Agent& agent) noexcept -> Result<Void> override;

  virtual auto
  OnDestroy(Agent& agent) noexcept -> void override;

  virtual auto
  OnUpdate(Agent& agent) noexcept -> void override;

  [[nodiscard]] auto
  GetConfig() const noexcept -> const Dict&;

  [[nodiscard]] auto
  GetConfig() noexcept -> Dict&;

  [[nodiscard]] static auto
  FromArgs(int argc, char** argv) noexcept -> Result<ServicePtr>;

 private:
  explicit ConfigService(Dict&& config) noexcept;

  Dict config_;
};

}  // namespace kero

#endif  // KERO_ENGINE_CONFIG_SERVICE_H
