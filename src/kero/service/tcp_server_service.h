#ifndef KERO_SERVICE_TCP_SERVER_SERVICE_H
#define KERO_SERVICE_TCP_SERVER_SERVICE_H

#include "kero/core/utils_linux.h"
#include "kero/service/service.h"

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
  OnEvent(Agent& agent, const std::string& event, const Dict& data) noexcept
      -> void override;

 private:
  Fd::Value server_fd_{Fd::kUnspecifiedInitialValue};
};

}  // namespace kero

#endif  // KERO_SERVICE_TCP_SERVER_SERVICE_H
