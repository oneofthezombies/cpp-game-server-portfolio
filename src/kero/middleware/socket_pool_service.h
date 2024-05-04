#ifndef KERO_MIDDLEWARE_SOCKET_POOL_SERVICE_H
#define KERO_MIDDLEWARE_SOCKET_POOL_SERVICE_H

#include <unordered_set>

#include "kero/engine/service.h"
#include "kero/middleware/common.h"

namespace kero {

class SocketPoolService : public Service {
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

  [[nodiscard]] auto
  RegisterSocket(const SocketId socket_id) noexcept -> Result<Void>;

  [[nodiscard]] auto
  UnregisterSocket(const SocketId socket_id,
                   std::string&& reason) noexcept -> Result<Void>;

 protected:
  std::unordered_set<SocketId> socket_ids_;
  std::unordered_map<
      std::string /* event */,
      std::function<Result<Void>(SocketPoolService*, const FlatJson&)>>
      event_handler_map_;
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_SOCKET_POOL_SERVICE_H
