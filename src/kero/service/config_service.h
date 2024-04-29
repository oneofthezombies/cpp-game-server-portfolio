#ifndef KERO_SERVICE_CONFIG_SERVICE_H
#define KERO_SERVICE_CONFIG_SERVICE_H

#include "kero/core/args_scanner.h"
#include "kero/core/common.h"
#include "kero/service/service.h"

namespace kero {

class ConfigService final : public Service {
 public:
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

 private:
  explicit ConfigService(Dict&& config) noexcept;

  Dict config_;

  friend class ConfigServiceFactory;
};

class ConfigServiceFactory final : public ServiceFactory {
 public:
  enum : Error::Code {
    kPortNotFound = 1,
    kPortParsingFailed,
    kUnknownArgument
  };

  explicit ConfigServiceFactory(int argc, char** argv) noexcept;
  virtual ~ConfigServiceFactory() noexcept override = default;
  CLASS_KIND_PINNABLE(ConfigServiceFactory);

  [[nodiscard]] virtual auto
  Create() noexcept -> Result<ServicePtr> override;

 private:
  Args args_;
};

}  // namespace kero

#endif  // KERO_SERVICE_CONFIG_SERVICE_H
