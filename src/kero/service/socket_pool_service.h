#ifndef KERO_SERVICE_SOCKET_POLL_SERVICE_H
#define KERO_SERVICE_SOCKET_POLL_SERVICE_H

#include <unordered_set>

#include "kero/core/utils_linux.h"
#include "kero/engine/service.h"

namespace kero {

class SocketPoolService final : public Service {
 public:
  explicit SocketPoolService(RunnerContextPtr&& runner_context) noexcept;
  virtual ~SocketPoolService() noexcept override = default;
  CLASS_KIND_MOVABLE(SocketPoolService);

  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void> override;

  virtual auto
  OnEvent(const std::string& event, const Dict& data) noexcept -> void override;

 private:
  auto
  OnSocketOpen(const Dict& data) noexcept -> void;
  auto
  OnSocketClose(const Dict& data) noexcept -> void;

  std::unordered_set<Fd::Value> sockets_;
};

}  // namespace kero

#endif  // KERO_SERVICE_SOCKET_POLL_SERVICE_H
