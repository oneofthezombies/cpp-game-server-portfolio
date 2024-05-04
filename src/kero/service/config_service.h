#ifndef KERO_SERVICE_CONFIG_SERVICE_H
#define KERO_SERVICE_CONFIG_SERVICE_H

#include "kero/core/args_scanner.h"
#include "kero/core/common.h"
#include "kero/engine/service.h"
#include "kero/engine/service_factory.h"

namespace kero {

class ConfigService final : public Service {
 public:
  explicit ConfigService(const Pin<RunnerContext> runner_context,
                         Json&& config) noexcept;
  virtual ~ConfigService() noexcept override = default;
  CLASS_KIND_MOVABLE(ConfigService);

  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void> override;

  virtual auto
  OnDestroy() noexcept -> void override;

  virtual auto
  OnUpdate() noexcept -> void override;

  [[nodiscard]] auto
  GetConfig() const noexcept -> const Json&;

  [[nodiscard]] auto
  GetConfig() noexcept -> Json&;

 private:
  Json config_;
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
  Create(const Pin<RunnerContext> runner_context) noexcept
      -> Result<Own<Service>> override;

 private:
  Args args_;
};

}  // namespace kero

#endif  // KERO_SERVICE_CONFIG_SERVICE_H
