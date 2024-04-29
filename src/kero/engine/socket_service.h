#ifndef KERO_ENGINE_SOCKET_SERVICE_H
#define KERO_ENGINE_SOCKET_SERVICE_H

#include <unordered_set>

#include "kero/core/utils_linux.h"
#include "kero/engine/service.h"

namespace kero {

class SocketService final : public Service {
 public:
  explicit SocketService() noexcept;
  virtual ~SocketService() noexcept override = default;
  CLASS_KIND_MOVABLE(SocketService);

  [[nodiscard]] virtual auto
  OnCreate(Agent& agent) noexcept -> Result<Void> override;

 private:
  std::unordered_set<Fd::Value> fds_;
};

}  // namespace kero

#endif  // KERO_ENGINE_SOCKET_SERVICE_H
