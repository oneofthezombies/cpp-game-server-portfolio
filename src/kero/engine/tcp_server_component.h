#ifndef KERO_ENGINE_TCP_SERVER_COMPONENT_H
#define KERO_ENGINE_TCP_SERVER_COMPONENT_H

#include "kero/engine/component.h"
#include "kero/engine/utils_linux.h"

namespace kero {

class TcpServerComponent final : public Component {
 public:
  explicit TcpServerComponent() noexcept;
  virtual ~TcpServerComponent() noexcept override = default;
  CLASS_KIND_MOVABLE(TcpServerComponent);

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

#endif  // KERO_ENGINE_TCP_SERVER_COMPONENT_H
