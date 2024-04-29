#ifndef KERO_ENGINE_TCP_SERVER_SERVICE_H
#define KERO_ENGINE_TCP_SERVER_SERVICE_H

#include "kero/engine/service.h"
#include "kero/engine/utils_linux.h"

namespace kero {

class TcpServerService final : public Service {
 public:
  explicit TcpServerService() noexcept;
  virtual ~TcpServerService() noexcept override = default;
  CLASS_KIND_MOVABLE(TcpServerService);

  [[nodiscard]] virtual auto
  OnCreate(Agent& agent) noexcept -> Result<Void> override;

  virtual auto
  OnDestroy(Agent& agent) noexcept -> void override;

  virtual auto
  OnUpdate(Agent& agent) noexcept -> void override;

 private:
  Fd::Value server_fd_{Fd::kUnspecifiedInitialValue};
};

}  // namespace kero

#endif  // KERO_ENGINE_TCP_SERVER_SERVICE_H
