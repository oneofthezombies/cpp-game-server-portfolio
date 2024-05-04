#ifndef KERO_MIDDLEWARE_SOCKET_ROUTER_SERVICE_H
#define KERO_MIDDLEWARE_SOCKET_ROUTER_SERVICE_H

#include "kero/engine/service.h"
#include "kero/middleware/common.h"

namespace kero {

class SocketRouterService final : public Service {
 public:
  explicit SocketRouterService(const Borrow<RunnerContext> runner_context,
                               std::string&& target) noexcept;
  virtual ~SocketRouterService() noexcept override = default;
  KERO_CLASS_KIND_MOVABLE(SocketRouterService);
  KERO_SERVICE_KIND(kServiceKindId_SocketRouter, "socket_router");

  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void> override;

  virtual auto
  OnEvent(const std::string& event,
          const FlatJson& data) noexcept -> void override;

 private:
  std::string target_;
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_SOCKET_ROUTER_SERVICE_H
