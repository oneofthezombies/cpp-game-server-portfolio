#ifndef KERO_MIDDLEWARE_TCP_SERVER_SERVICE_H
#define KERO_MIDDLEWARE_TCP_SERVER_SERVICE_H

#include "kero/core/utils_linux.h"
#include "kero/engine/service.h"
#include "kero/middleware/common.h"

namespace kero {

class TcpServerService final : public Service {
 public:
  explicit TcpServerService(const Pin<RunnerContext> runner_context) noexcept;
  virtual ~TcpServerService() noexcept override = default;
  KERO_CLASS_KIND_MOVABLE(TcpServerService);
  KERO_SERVICE_KIND(kServiceKindId_TcpServer, "tcp_server");

  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void> override;

  virtual auto
  OnDestroy() noexcept -> void override;

  virtual auto
  OnEvent(const std::string& event,
          const FlatJson& data) noexcept -> void override;

 private:
  Fd::Value server_fd_{Fd::kUnspecifiedInitialValue};
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_TCP_SERVER_SERVICE_H
