#ifndef KERO_SERVICE_SOCKET_ROUTER_SERVICE_H
#define KERO_SERVICE_SOCKET_ROUTER_SERVICE_H

#include "kero/service/service.h"

namespace kero {

class SocketRouterService final : public Service {
  explicit SocketRouterService() noexcept;
  virtual ~SocketRouterService() noexcept override = default;
  CLASS_KIND_MOVABLE(SocketRouterService);

  [[nodiscard]] virtual auto
  OnCreate(Agent& agent) noexcept -> Result<Void> override;

  virtual auto
  OnEvent(Agent& agent, const std::string& event, const Dict& data) noexcept
      -> void override;

 private:
  std::string target_actor_;
};

}  // namespace kero

#endif  // KERO_SERVICE_SOCKET_ROUTER_SERVICE_H
