#include "lobby_event_loop_linux.h"

#include <iostream>

#include <sys/epoll.h>

#include "core/tiny_json.h"

#include "event_loop_linux.h"
#include "file_descriptor_linux.h"

auto LobbyEventLoopLinux::OnMailReceived(const Mail &mail) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto found = mail.body.find("client_fd"); found != mail.body.end()) {
    const auto &client_fd_str = found->second;
    auto client_fd_res =
        ParseNumberString<FileDescriptorLinux::Raw>(client_fd_str);
    if (client_fd_res.IsErr()) {
      return ResultT{Error{
          Symbol::kLobbyEventLoopLinuxClientFdConversionFailed,
          TinyJson{}
              .Set("client_fd_str", client_fd_str)
              .Set("errc", std::make_error_code(client_fd_res.Err()).message())
              .ToString()}};
    }

    const auto client_fd_raw = client_fd_res.Ok();
    if (auto res = AddClientFd(client_fd_raw); res.IsErr()) {
      return res;
    }

    return OnClientFdInserted(client_fd_raw);
  }

  return ResultT{Void{}};
}

auto LobbyEventLoopLinux::OnEpollEventReceived(
    const struct epoll_event &event) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  return ResultT{Void{}};
}

auto LobbyEventLoopLinux::OnClientFdInserted(
    const FileDescriptorLinux::Raw client_fd_raw) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (client_fds_.size() >= 2) {
    const auto client_fd_0 = *client_fds_.begin();
    const auto client_fd_1 = *std::next(client_fds_.begin());
    if (auto res = OnClientFdMatched(client_fd_0, client_fd_1); res.IsErr()) {
      return res;
    }
  }

  return ResultT{Void{}};
}

auto LobbyEventLoopLinux::OnClientFdMatched(
    const FileDescriptorLinux::Raw client_fd_0,
    const FileDescriptorLinux::Raw client_fd_1) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  AssertInit();
  if (auto res = DeleteClientFd(client_fd_0); res.IsErr()) {
    return res;
  }

  if (auto res = DeleteClientFd(client_fd_1); res.IsErr()) {
    return res;
  }

  std::cout << "Matched " << client_fd_0 << " and " << client_fd_1 << std::endl;
  context_->mail_box.tx.Send(
      Mail{"lobby",
           "battle",
           {{"matched_client_fds", TinyJson{}
                                       .Set("client_fd_0", client_fd_0)
                                       .Set("client_fd_1", client_fd_1)
                                       .ToString()}}});

  return ResultT{Void{}};
}

auto LobbyEventLoopLinux::AddClientFd(
    const FileDescriptorLinux::Raw client_fd) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = Add(client_fd, EPOLLIN | EPOLLET); res.IsErr()) {
    return res;
  }

  if (client_fds_.find(client_fd) != client_fds_.end()) {
    const Error error{Symbol::kLobbyEventLoopLinuxClientFdAlreadyExists,
                      TinyJson{}
                          .Set("reason", "Delete old client fd")
                          .Set("client_fd", client_fd)
                          .ToString()};
    std::cout << error << std::endl;
    client_fds_.erase(client_fd);
  }

  client_fds_.insert(client_fd);
  return ResultT{Void{}};
}

auto LobbyEventLoopLinux::DeleteClientFd(
    const FileDescriptorLinux::Raw client_fd) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  client_fds_.erase(client_fd);
  if (auto res = Delete(client_fd); res.IsErr()) {
    return res;
  }

  return ResultT{Void{}};
}
