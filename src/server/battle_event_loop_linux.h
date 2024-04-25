#ifndef SERVER_BATTLE_EVENT_LOOP_LINUX_H
#define SERVER_BATTLE_EVENT_LOOP_LINUX_H

#include "core/core.h"

#include "event_loop_linux.h"

class BattleEventLoopLinux final : private core::NonCopyable, core::Movable {
public:
  using RoomId = uint64_t;
  using EventHandler = std::function<Result<core::Void>(const std::string &)>;

  [[nodiscard]] auto Run() noexcept -> Result<core::Void>;

private:
  explicit BattleEventLoopLinux(
      EventLoopLinux &&event_loop,
      core::Tx<EventLoopLinuxEvent> &&battle_to_lobby_tx) noexcept;

  [[nodiscard]] auto OnEventLoopEvent(const EventLoopLinuxEvent &event) noexcept
      -> Result<core::Void>;
  [[nodiscard]] auto OnEpollEvent(const struct epoll_event &event) noexcept
      -> Result<core::Void>;

  [[nodiscard]] auto AddClientFd(const FileDescriptorLinux::Raw client_fd,
                                 const RoomId room_id) noexcept
      -> Result<core::Void>;
  [[nodiscard]] auto
  DeleteClientFd(const FileDescriptorLinux::Raw client_fd) noexcept
      -> Result<core::Void>;

  [[nodiscard]] auto NextRoomId() noexcept -> RoomId;

  [[nodiscard]] auto
  OnMatchedClientFds(const std::string &matched_client_fds) noexcept
      -> Result<core::Void>;

  EventLoopLinux event_loop_;
  core::Tx<EventLoopLinuxEvent> battle_to_lobby_tx_;
  std::unordered_map<FileDescriptorLinux::Raw, RoomId> client_fds_;
  std::unordered_map<std::string, EventHandler> event_handlers_;
  RoomId next_room_id_{};

  friend class BattleEventLoopLinuxBuilder;
};

class BattleEventLoopLinuxBuilder final : private core::NonCopyable,
                                          private core::NonMovable {
public:
  [[nodiscard]] auto
  Build(core::Rx<EventLoopLinuxEvent> &&lobby_to_battle_rx,
        core::Tx<EventLoopLinuxEvent> &&battle_to_lobby_tx) const noexcept
      -> Result<BattleEventLoopLinux>;
};

#endif // SERVER_BATTLE_EVENT_LOOP_LINUX_H
