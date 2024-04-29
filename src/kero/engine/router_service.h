#ifndef KERO_ENGINE_ROUTER_SERVICE_H
#define KERO_ENGINE_ROUTER_SERVICE_H

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
};

}  // namespace kero

#endif  // KERO_ENGINE_ROUTER_SERVICE_H
