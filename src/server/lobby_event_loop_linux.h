#ifndef SERVER_LOBBY_EVENT_LOOP_LINUX_H
#define SERVER_LOBBY_EVENT_LOOP_LINUX_H

#include <unordered_set>

#include "core/core.h"

#include "event_loop_linux.h"

class LobbyEventLoopLinux final : public EventLoopLinux {
public:
  using Super = EventLoopLinux;

  explicit LobbyEventLoopLinux() noexcept = default;
  ~LobbyEventLoopLinux() noexcept override = default;
  CLASS_KIND_MOVABLE(LobbyEventLoopLinux);

private:
  [[nodiscard]] auto OnMailReceived(const Mail &mail) noexcept
      -> Result<Void> override;

  [[nodiscard]] auto
  OnEpollEventReceived(const struct epoll_event &event) noexcept
      -> Result<Void> override;

  [[nodiscard]] auto
  OnClientFdInserted(const FileDescriptorLinux::Raw client_fd_raw) noexcept
      -> Result<Void>;

  [[nodiscard]] auto
  OnClientFdMatched(const FileDescriptorLinux::Raw client_fd_0,
                    const FileDescriptorLinux::Raw client_fd_1) noexcept
      -> Result<Void>;

  [[nodiscard]] auto
  AddClientFd(const FileDescriptorLinux::Raw client_fd) noexcept
      -> Result<Void>;
  [[nodiscard]] auto
  DeleteClientFd(const FileDescriptorLinux::Raw client_fd) noexcept
      -> Result<Void>;

  std::unordered_set<FileDescriptorLinux::Raw> client_fds_;
};

#endif // SERVER_LOBBY_EVENT_LOOP_LINUX_H
