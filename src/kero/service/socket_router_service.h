#ifndef KERO_SERVICE_SOCKET_ROUTER_SERVICE_H
#define KERO_SERVICE_SOCKET_ROUTER_SERVICE_H

#include "kero/engine/service.h"

namespace kero {

class SocketRouterService final : public Service {
 public:
  explicit SocketRouterService(
      const Pin<RunnerContext> runner_context) noexcept;
  virtual ~SocketRouterService() noexcept override = default;
  CLASS_KIND_MOVABLE(SocketRouterService);

  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void> override;

  virtual auto
  OnEvent(const std::string& event, const Json& data) noexcept -> void override;

 private:
  std::string target_actor_;
};

}  // namespace kero

#endif  // KERO_SERVICE_SOCKET_ROUTER_SERVICE_H
