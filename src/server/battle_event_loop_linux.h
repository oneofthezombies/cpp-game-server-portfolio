#ifndef SERVER_BATTLE_EVENT_LOOP_LINUX_H
#define SERVER_BATTLE_EVENT_LOOP_LINUX_H

#include "core/core.h"

#include "event_loop_linux.h"

class BattleEventLoopLinux final : public EventLoopLinux {
public:
  using Super = EventLoopLinux;
  using RoomId = uint64_t;
  using EventHandler = std::function<Result<Void>(const std::string &)>;

  explicit BattleEventLoopLinux() noexcept;
  ~BattleEventLoopLinux() noexcept override = default;
  CLASS_KIND_MOVABLE(BattleEventLoopLinux);

private:
  [[nodiscard]] auto OnMailReceived(const Mail &mail) noexcept
      -> Result<Void> override;

  [[nodiscard]] auto
  OnEpollEventReceived(const struct epoll_event &event) noexcept
      -> Result<Void> override;

  [[nodiscard]] auto AddClientFd(const FileDescriptorLinux::Raw client_fd,
                                 const RoomId room_id) noexcept -> Result<Void>;
  [[nodiscard]] auto
  DeleteClientFd(const FileDescriptorLinux::Raw client_fd) noexcept
      -> Result<Void>;

  [[nodiscard]] auto NextRoomId() noexcept -> RoomId;

  [[nodiscard]] auto
  OnMatchedClientFds(const std::string &matched_client_fds) noexcept
      -> Result<Void>;

  std::unordered_map<FileDescriptorLinux::Raw, RoomId> client_fds_;
  std::unordered_map<std::string, EventHandler> event_handlers_;
  RoomId next_room_id_{};
};

#endif // SERVER_BATTLE_EVENT_LOOP_LINUX_H
