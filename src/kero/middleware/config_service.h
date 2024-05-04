#ifndef KERO_MIDDLEWARE_CONFIG_SERVICE_H
#define KERO_MIDDLEWARE_CONFIG_SERVICE_H

#include "kero/core/args_scanner.h"
#include "kero/core/common.h"
#include "kero/engine/service.h"
#include "kero/engine/service_factory.h"
#include "kero/middleware/common.h"

namespace kero {

class ConfigService final : public Service {
 public:
  explicit ConfigService(const Borrow<RunnerContext> runner_context,
                         FlatJson&& config) noexcept;
  virtual ~ConfigService() noexcept override = default;
  KERO_CLASS_KIND_MOVABLE(ConfigService);
  KERO_SERVICE_KIND(kServiceKindId_Config, "config");

  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void> override;

  [[nodiscard]] auto
  GetConfig() const noexcept -> const FlatJson&;

  [[nodiscard]] auto
  GetConfig() noexcept -> FlatJson&;

 private:
  FlatJson config_;
};

class ConfigServiceFactory : public ServiceFactory {
 public:
  enum : Error::Code {
    kPortNotFound = 1,
    kPortParsingFailed,
    kUnknownArgument
  };

  explicit ConfigServiceFactory(int argc, char** argv) noexcept;
  virtual ~ConfigServiceFactory() noexcept override = default;

  [[nodiscard]] virtual auto
  Create(const Borrow<RunnerContext> runner_context) noexcept
      -> Result<Own<Service>> override;

 private:
  Args args_;
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_CONFIG_SERVICE_H
