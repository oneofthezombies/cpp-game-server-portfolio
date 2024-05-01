#ifndef KERO_SERVICE_CONFIG_SERVICE_H
#define KERO_SERVICE_CONFIG_SERVICE_H

#include "kero/core/args_scanner.h"
#include "kero/core/common.h"
#include "kero/engine/service.h"

namespace kero {

class ConfigService final : public Service {
 public:
  virtual ~ConfigService() noexcept override = default;
  CLASS_KIND_MOVABLE(ConfigService);

  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void> override;

  virtual auto
  OnDestroy() noexcept -> void override;

  virtual auto
  OnUpdate() noexcept -> void override;

  [[nodiscard]] auto
  GetConfig() const noexcept -> const Dict&;

  [[nodiscard]] auto
  GetConfig() noexcept -> Dict&;

 private:
  explicit ConfigService(const Pin<RunnerContext> runner_context,
                         Dict&& config) noexcept;

  Dict config_;

  friend class ConfigServiceFactory;
};

class ConfigServiceFactoryProvider final {
 public:
  enum : Error::Code {
    kPortNotFound = 1,
    kPortParsingFailed,
    kUnknownArgument
  };

  explicit ConfigServiceFactoryProvider(int argc, char** argv) noexcept;
  ~ConfigServiceFactoryProvider() noexcept = default;
  CLASS_KIND_PINNABLE(ConfigServiceFactoryProvider);

  [[nodiscard]] auto
  Create() noexcept -> ServiceFactory;

 private:
  Args args_;
};

}  // namespace kero

#endif  // KERO_SERVICE_CONFIG_SERVICE_H
