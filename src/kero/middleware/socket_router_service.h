#ifndef KERO_MIDDLEWARE_SOCKET_ROUTER_SERVICE_H
#define KERO_MIDDLEWARE_SOCKET_ROUTER_SERVICE_H

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
  OnEvent(const std::string& event,
          const FlatJson& data) noexcept -> void override;

  [[nodiscard]] static auto
  GetKindId() noexcept -> ServiceKindId;

  [[nodiscard]] static auto
  GetKindName() noexcept -> ServiceKindName;

 private:
  std::string target_actor_;
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_SOCKET_ROUTER_SERVICE_H
