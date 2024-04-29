#ifndef KERO_SERVICE_IO_EVENT_LOOP_SERVICE_H
#define KERO_SERVICE_IO_EVENT_LOOP_SERVICE_H

#include "kero/core/utils_linux.h"
#include "kero/service/service.h"

struct epoll_event;

namespace kero {

class IoEventLoopService final : public Service {
 public:
  enum : Error::Code { kInvalidEpollFd = 1, kSocketClosed };

  struct AddOptions {
    bool in{false};
    bool out{false};
    bool edge_trigger{false};
  };

  explicit IoEventLoopService() noexcept;
  virtual ~IoEventLoopService() noexcept override = default;
  CLASS_KIND_MOVABLE(IoEventLoopService);

  [[nodiscard]] virtual auto
  OnCreate(Agent& agent) noexcept -> Result<Void> override;

  virtual auto
  OnDestroy(Agent& agent) noexcept -> void override;

  virtual auto
  OnUpdate(Agent& agent) noexcept -> void override;

  [[nodiscard]] auto
  AddFd(const Fd::Value fd, const AddOptions options) const noexcept
      -> Result<Void>;

  [[nodiscard]] auto
  RemoveFd(const Fd::Value fd) const noexcept -> Result<Void>;

  [[nodiscard]] auto
  WriteToFd(const Fd::Value fd, const std::string_view data) const noexcept
      -> Result<Void>;

  [[nodiscard]] auto
  ReadFromFd(Agent& agent, const Fd::Value fd) const noexcept
      -> Result<std::string>;

 private:
  auto
  OnUpdateEpollEvent(Agent& agent, const struct ::epoll_event& event) noexcept
      -> Result<Void>;

  Fd::Value epoll_fd_{Fd::kUnspecifiedInitialValue};

  static constexpr size_t kMaxEvents = 1024;
};

}  // namespace kero

#endif  // KERO_SERVICE_IO_EVENT_LOOP_SERVICE_H
