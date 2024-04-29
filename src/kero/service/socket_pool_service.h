#ifndef KERO_SERVICE_SOCKET_POLL_SERVICE_H
#define KERO_SERVICE_SOCKET_POLL_SERVICE_H

#include <unordered_set>

#include "kero/core/utils_linux.h"
#include "kero/service/service.h"

namespace kero {

class SocketPoolService final : public Service {
 public:
  explicit SocketPoolService() noexcept;
  virtual ~SocketPoolService() noexcept override = default;
  CLASS_KIND_MOVABLE(SocketPoolService);

  [[nodiscard]] virtual auto
  OnCreate(Agent& agent) noexcept -> Result<Void> override;

  virtual auto
  OnEvent(Agent& agent, const std::string& event, const Dict& data) noexcept
      -> void override;

 private:
  auto
  OnSocketOpen(Agent& agent, const Dict& data) noexcept -> void;
  auto
  OnSocketClose(Agent& agent, const Dict& data) noexcept -> void;

  std::unordered_set<Fd::Value> sockets_;
};

}  // namespace kero

#endif  // KERO_SERVICE_SOCKET_POLL_SERVICE_H
