#ifndef KERO_MIDDLEWARE_SOCKET_POLL_SERVICE_H
#define KERO_MIDDLEWARE_SOCKET_POLL_SERVICE_H

#include <unordered_set>

#include "kero/core/utils_linux.h"
#include "kero/engine/service.h"

namespace kero {

class SocketPoolService final : public Service {
 public:
  explicit SocketPoolService(const Pin<RunnerContext> runner_context) noexcept;
  virtual ~SocketPoolService() noexcept override = default;
  CLASS_KIND_MOVABLE(SocketPoolService);

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
  auto
  OnSocketOpen(const FlatJson& data) noexcept -> void;
  auto
  OnSocketClose(const FlatJson& data) noexcept -> void;

  std::unordered_set<Fd::Value> sockets_;
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_SOCKET_POLL_SERVICE_H
