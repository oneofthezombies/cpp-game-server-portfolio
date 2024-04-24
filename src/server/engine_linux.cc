#include "engine_linux.h"

#include <atomic>
#include <cassert>
#include <iostream>
#include <signal.h>

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "common.h"
#include "core/protocol.h"
#include "file_descriptor_linux.h"
#include "session.h"
#include "utils.h"
#include "utils_linux.h"

std::atomic<bool> interrupted{false};

auto OnSignal(int signal) -> void { interrupted = true; }

LinuxEngine::LinuxEngine(ThreadWorkerPool &&thread_worker_pool,
                         LinuxFileDescriptor &&epoll_fd,
                         LinuxFileDescriptor &&server_fd) noexcept
    : thread_worker_pool_{std::move(thread_worker_pool)},
      epoll_fd_{std::move(epoll_fd)}, server_fd_{std::move(server_fd)} {
  assert(epoll_fd_.IsValid() && "Invalid epoll file descriptor");
  assert(server_fd_.IsValid() && "Invalid server file descriptor");
}

auto LinuxEngine::Run() noexcept -> ResultMany<core::Void> {
  struct epoll_event add_server_ev {};
  add_server_ev.events = EPOLLIN;
  add_server_ev.data.fd = server_fd_.AsRaw();
  if (epoll_ctl(epoll_fd_.AsRaw(), EPOLL_CTL_ADD, server_fd_.AsRaw(),
                &add_server_ev) == -1) {
    std::vector<Error> errors;
    errors.emplace_back(Error{Symbol::kLinuxEngineEpollCtlAddServerFailed,
                              SB{}.Add(core::LinuxError::FromErrno()).Build()});
    return errors;
  }

  if (signal(SIGINT, OnSignal) == SIG_ERR) {
    std::vector<Error> errors;
    errors.emplace_back(Error{Symbol::kLinuxSignalSetFailed,
                              SB{}.Add(core::LinuxError::FromErrno()).Build()});
    return errors;
  }

  struct epoll_event events[kMaxEvents]{};
  while (!interrupted) {
    const int nfds = epoll_wait(epoll_fd_.AsRaw(), events, kMaxEvents, -1);
    if (nfds == -1) {
      if (errno == EINTR) {
        continue;
      }

      std::vector<Error> errors;
      errors.emplace_back(
          Error{Symbol::kLinuxEngineEpollWaitFailed,
                SB{}.Add(core::LinuxError::FromErrno()).Build()});
      return errors;
    }

    for (int n = 0; n < nfds; ++n) {
      const auto &current_ev = events[n];
      if (current_ev.data.fd == server_fd_.AsRaw()) {
        if (auto res = OnServerFdEvent(); res.IsErr()) {
          std::cout << res.Err() << std::endl;
        }
      } else {
        if (auto res = OnClientFdEvent(current_ev.data.fd); res.IsErr()) {
          std::cout << res.Err() << std::endl;
        }
      }
    }
  }

  if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
    std::vector<Error> errors;
    errors.emplace_back(Error{Symbol::kLinuxSignalResetFailed,
                              SB{}.Add(core::LinuxError::FromErrno()).Build()});
    return errors;
  }

  if (auto res = thread_worker_pool_.Shutdown(); res.IsErr()) {
    return std::move(res.Err());
  }

  return core::Void{};
}

auto LinuxEngine::OnServerFdEvent() noexcept -> Result<core::Void> {
  struct sockaddr_in client_addr {};
  socklen_t addrlen = sizeof(struct sockaddr_in);
  auto client_fd = LinuxFileDescriptor{
      accept(server_fd_.AsRaw(), (struct sockaddr *)&client_addr, &addrlen)};
  if (!client_fd.IsValid()) {
    return Error{Symbol::kLinuxEngineServerSocketAcceptFailed,
                 SB{}.Add(core::LinuxError::FromErrno()).Build()};
  }

  if (auto res = client_fd.UpdateNonBlocking(); res.IsErr()) {
    return std::move(res.Err());
  }

  // Add client fd to epoll
  {
    struct epoll_event add_client_ev {};
    add_client_ev.events = EPOLLIN | EPOLLET;
    add_client_ev.data.fd = client_fd.AsRaw();
    if (epoll_ctl(epoll_fd_.AsRaw(), EPOLL_CTL_ADD, client_fd.AsRaw(),
                  &add_client_ev) == -1) {
      return Error{Symbol::kLinuxEngineEpollCtlAddClientFailed,
                   SB{}.Add(core::LinuxError::FromErrno())
                       .Add("client_fd", client_fd)
                       .Build()};
    }
  }

  // Add session to the map
  {
    Session session{
        SessionImplPtr{new LinuxFileDescriptor{std::move(client_fd)}}};
    if (auto old_session = connected_sessions_.find(session.Id());
        old_session != connected_sessions_.end()) {
      const Error error{Symbol::kLinuxEngineSessionAlreadyExists,
                        SB{}.Add("Delete old session")
                            .Add("old_session", old_session->second)
                            .Build()};
      std::cout << error << std::endl;

      connected_sessions_.erase(old_session);
    }

    connected_sessions_.emplace(session.Id(), std::move(session));
  }

  return core::Void{};
}

auto LinuxEngine::OnClientFdEvent(
    const LinuxFileDescriptor::Raw client_fd) noexcept -> Result<core::Void> {
  char buffer[1024 * 8]{};
  const auto count = read(client_fd, buffer, sizeof(buffer));
  if (count == -1) {
    DeleteConnectedSessionOrCloseFd(client_fd);
    return Error{Symbol::kLinuxEngineClientSocketReadFailed,
                 SB{}.Add(core::LinuxError::FromErrno())
                     .Add("client_fd", client_fd)
                     .Build()};
  }

  if (count == 0) {
    DeleteConnectedSessionOrCloseFd(client_fd);
    return Error{Symbol::kLinuxEngineClientSocketClosed,
                 SB{}.Add("client_fd", client_fd).Build()};
  }

  // remove from epoll
  {
    struct epoll_event del_client_ev {};
    del_client_ev.events = EPOLLIN | EPOLLET;
    del_client_ev.data.fd = client_fd;
    if (epoll_ctl(epoll_fd_.AsRaw(), EPOLL_CTL_DEL, client_fd,
                  &del_client_ev) == -1) {
      return Error{Symbol::kLinuxEngineEpollCtlDeleteClientFailed,
                   SB{}.Add(core::LinuxError::FromErrno())
                       .Add("client_fd", client_fd)
                       .Build()};
    }
  }

  // parse message
  const std::string_view buffer_view{buffer, static_cast<size_t>(count)};
  const auto message = core::Message::FromRaw(buffer_view);
  if (!message) {
    DeleteConnectedSessionOrCloseFd(client_fd);
    return Error{Symbol::kLinuxEngineMessageParseFailed,
                 SB{}.Add("buffer_view", buffer_view)
                     .Add("client_fd", client_fd)
                     .Build()};
  }

  const auto room_id = message->json.Get("room_id");
  if (!room_id) {
    DeleteConnectedSessionOrCloseFd(client_fd);
    return Error{Symbol::kLinuxEngineMessageParseFailed,
                 SB{}.Add("room_id not found")
                     .Add("buffer_view", buffer_view)
                     .Add("client_fd", client_fd)
                     .Build()};
  }

  // write to client_fd message
  {
    const auto success =
        core::Message::BuildRaw(core::MessageKind::kRequestSuccess, message->id,
                                core::TinyJsonBuilder{}.Build());
    if (write(client_fd, success.data(), success.size()) == -1) {
      DeleteConnectedSessionOrCloseFd(client_fd);
      return Error{Symbol::kLinuxEngineClientSocketWriteFailed,
                   SB{}.Add(core::LinuxError::FromErrno())
                       .Add("client_fd", client_fd)
                       .Build()};
    }
  }

  return core::Void{};
}

auto LinuxEngine::DeleteConnectedSessionOrCloseFd(
    const LinuxFileDescriptor::Raw client_fd) noexcept -> void {
  const auto session_id = LinuxFileDescriptor::RawToSessionId(client_fd);
  auto found_session = connected_sessions_.find(session_id);
  if (found_session != connected_sessions_.end()) {
    connected_sessions_.erase(found_session);
    return;
  }

  if (close(client_fd) == -1) {
    const Error error{Symbol::kLinuxEngineClientSocketCloseFailed,
                      SB{}.Add(core::LinuxError::FromErrno())
                          .Add("client_fd", client_fd)
                          .Build()};
    std::cout << error << std::endl;
  }
}

auto LinuxEngineBuilder::Build(
    const uint16_t port, ThreadWorkerPool &&thread_worker_pool) const noexcept
    -> Result<LinuxEngine> {
  auto epoll_fd = LinuxFileDescriptor{epoll_create1(0)};
  if (!epoll_fd.IsValid()) {
    return Error{Symbol::kLinuxEngineEpollCreate1Failed,
                 SB{}.Add(core::LinuxError::FromErrno()).Build()};
  }

  auto server_fd = LinuxFileDescriptor{socket(AF_INET, SOCK_STREAM, 0)};
  if (!server_fd.IsValid()) {
    return Error{Symbol::kLinuxEngineServerSocketFailed,
                 SB{}.Add(core::LinuxError::FromErrno()).Build()};
  }

  if (auto result = server_fd.UpdateNonBlocking(); result.IsErr()) {
    return Error{std::move(result.Err())};
  }

  struct sockaddr_in server_addr {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_fd.AsRaw(), (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    return Error{Symbol::kLinuxEngineServerSocketBindFailed,
                 SB{}.Add(core::LinuxError::FromErrno()).Build()};
  }

  if (listen(server_fd.AsRaw(), SOMAXCONN) < 0) {
    return Error{Symbol::kLinuxEngineServerSocketListenFailed,
                 SB{}.Add(core::LinuxError::FromErrno()).Build()};
  }

  return LinuxEngine{std::move(thread_worker_pool), std::move(epoll_fd),
                     std::move(server_fd)};
}
