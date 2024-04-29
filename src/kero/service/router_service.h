#ifndef KERO_SERVICE_ROUTER_SERVICE_H
#define KERO_SERVICE_ROUTER_SERVICE_H

#include "kero/engine/service.h"

namespace kero {

class RouterService final : public Service {
  explicit RouterService() noexcept;
  virtual ~RouterService() noexcept override = default;
  CLASS_KIND_MOVABLE(RouterService);

  [[nodiscard]] virtual auto
  OnCreate(Agent& agent) noexcept -> Result<Void> override;

  virtual auto
  OnEvent(Agent& agent, const std::string& event, const Dict& data) noexcept
      -> void override;

 private:
  std::string routing_target_actor_;
};

}  // namespace kero

#endif  // KERO_SERVICE_ROUTER_SERVICE_H
