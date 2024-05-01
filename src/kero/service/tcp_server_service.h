#ifndef KERO_SERVICE_TCP_SERVER_SERVICE_H
#define KERO_SERVICE_TCP_SERVER_SERVICE_H

#include "kero/core/utils_linux.h"
#include "kero/engine/service.h"

namespace kero {

class TcpServerService final : public Service {
 public:
  explicit TcpServerService(RunnerContextPtr&& runner_context) noexcept;
  virtual ~TcpServerService() noexcept override = default;
  CLASS_KIND_MOVABLE(TcpServerService);

  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void> override;

  virtual auto
  OnDestroy() noexcept -> void override;

  virtual auto
  OnEvent(const std::string& event, const Dict& data) noexcept -> void override;

 private:
  Fd::Value server_fd_{Fd::kUnspecifiedInitialValue};
};

}  // namespace kero

#endif  // KERO_SERVICE_TCP_SERVER_SERVICE_H
