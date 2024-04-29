#ifndef KERO_ENGINE_IO_EVENT_LOOP_COMPONENT_H
#define KERO_ENGINE_IO_EVENT_LOOP_COMPONENT_H

#include "kero/engine/component.h"
#include "kero/engine/utils_linux.h"

struct epoll_event;

namespace kero {

class IoEventLoopComponent final : public Component {
 public:
  enum : Error::Code { kInvalidEpollFd = 1 };

  struct AddOptions {
    bool in{false};
    bool out{false};
    bool edge_trigger{false};
  };

  explicit IoEventLoopComponent() noexcept;
  virtual ~IoEventLoopComponent() noexcept override = default;
  CLASS_KIND_MOVABLE(IoEventLoopComponent);

  [[nodiscard]] virtual auto
  OnCreate(Agent& agent) noexcept -> Result<Void> override;

  virtual auto
  OnDestroy(Agent& agent) noexcept -> void override;

  virtual auto
  OnUpdate(Agent& agent) noexcept -> void override;

  [[nodiscard]] auto
  AddFd(const Fd::Value fd, const AddOptions options) noexcept -> Result<Void>;

 private:
  auto
  OnUpdateEpollEvent(const struct ::epoll_event& event) noexcept
      -> Result<Void>;

  Fd::Value epoll_fd_{Fd::kUnspecifiedInitialValue};

  static constexpr size_t kMaxEvents = 1024;
};

}  // namespace kero

#endif  // KERO_ENGINE_IO_EVENT_LOOP_COMPONENT_H
