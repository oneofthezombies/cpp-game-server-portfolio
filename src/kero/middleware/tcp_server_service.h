#ifndef KERO_MIDDLEWARE_TCP_SERVER_SERVICE_H
#define KERO_MIDDLEWARE_TCP_SERVER_SERVICE_H

#include "kero/core/utils_linux.h"
#include "kero/engine/service.h"

namespace kero {

class TcpServerService final : public Service {
 public:
  explicit TcpServerService(const Pin<RunnerContext> runner_context) noexcept;
  virtual ~TcpServerService() noexcept override = default;
  CLASS_KIND_MOVABLE(TcpServerService);

  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void> override;

  virtual auto
  OnDestroy() noexcept -> void override;

  virtual auto
  OnEvent(const std::string& event,
          const FlatJson& data) noexcept -> void override;

  [[nodiscard]] static auto
  GetKindId() noexcept -> ServiceKindId;

  [[nodiscard]] static auto
  GetKindName() noexcept -> ServiceKindName;

 private:
  Fd::Value server_fd_{Fd::kUnspecifiedInitialValue};
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_TCP_SERVER_SERVICE_H
