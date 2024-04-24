#include "engine_linux.h"

#include <atomic>
#include <cassert>
#include <cstring>
#include <signal.h>

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "common.h"
#include "server/file_descriptor_linux.h"

std::atomic<bool> interrupted{false};

auto OnSignal(int signal) -> void { interrupted = true; }

LinuxEngine::LinuxEngine(LinuxFileDescriptor &&epoll_fd,
                         LinuxFileDescriptor &&server_fd) noexcept
    : epoll_fd_{std::move(epoll_fd)}, server_fd_{std::move(server_fd)} {
  assert(epoll_fd_.IsValid() && "Invalid epoll file descriptor");
  assert(server_fd_.IsValid() && "Invalid server file descriptor");
}

auto LinuxEngine::Run() noexcept -> Result<core::Void> {
  struct epoll_event add_server_ev {};
  add_server_ev.events = EPOLLIN;
  add_server_ev.data.fd = server_fd_.AsRaw();
  if (epoll_ctl(epoll_fd_.AsRaw(), EPOLL_CTL_ADD, server_fd_.AsRaw(),
                &add_server_ev) == -1) {
    return Error{Symbol::kLinuxEngineEpollCtlAddServerFailed,
                 std::string{strerror(errno)}};
  }

  if (signal(SIGINT, OnSignal) == SIG_ERR) {
    return Error{Symbol::kLinuxSignalSetFailed, std::string{strerror(errno)}};
  }

  struct epoll_event events[kMaxEvents]{};
  while (!interrupted) {
    const int nfds = epoll_wait(epoll_fd_.AsRaw(), events, kMaxEvents, -1);
    if (nfds == -1) {
      if (errno == EINTR) {
        continue;
      }
      return Error{Symbol::kLinuxEngineEpollWaitFailed,
                   std::string{strerror(errno)}};
    }

    for (int n = 0; n < nfds; ++n) {
      const auto &current_ev = events[n];
      if (current_ev.data.fd == server_fd_.AsRaw()) {
        struct sockaddr_in client_addr {};
        socklen_t addrlen = sizeof(struct sockaddr_in);
        auto client_fd = LinuxFileDescriptor{accept(
            server_fd_.AsRaw(), (struct sockaddr *)&client_addr, &addrlen)};
        if (!client_fd.IsValid()) {
          const Error error{Symbol::kLinuxEngineServerSocketAcceptFailed,
                            std::string{strerror(errno)}};
          // TODO: Log error
          continue;
        }

        if (auto res = client_fd.UpdateNonBlocking(); res.IsErr()) {
          // TODO: Log error
          continue;
        }

        struct epoll_event add_client_ev {};
        add_client_ev.events = EPOLLIN | EPOLLET;
        add_client_ev.data.fd = client_fd.AsRaw();
        if (epoll_ctl(epoll_fd_.AsRaw(), EPOLL_CTL_ADD, client_fd.AsRaw(),
                      &add_client_ev) == -1) {
          const Error error{Symbol::kLinuxEngineEpollCtlAddClientFailed,
                            std::string{strerror(errno)}};
          // TODO: Log error
          continue;
        }
      } else {
        char buffer[1024]{};
        const ssize_t count = read(current_ev.data.fd, buffer, sizeof(buffer));
        if (count == -1) {
          const Error error{Symbol::kLinuxEngineClientSocketReadFailed,
                            std::string{strerror(errno)}};
          // TODO: Log error
          close(current_ev.data.fd);
        } else if (count == 0) {
          close(current_ev.data.fd);
        } else {
          if (write(current_ev.data.fd, buffer, count) == -1) {
            const Error error{Symbol::kLinuxEngineClientSocketReadFailed,
                              std::string{strerror(errno)}};
            // TODO: Log error
          }
        }
      }
    }
  }

  if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
    return Error{Symbol::kLinuxSignalResetFailed, std::string{strerror(errno)}};
  }

  return core::Void{};
}

auto LinuxEngineBuilder::Build(const uint16_t port) const noexcept
    -> Result<LinuxEngine> {
  auto epoll_fd = LinuxFileDescriptor{epoll_create1(0)};
  if (!epoll_fd.IsValid()) {
    return Error{Symbol::kLinuxEngineEpollCreate1Failed,
                 std::string{strerror(errno)}};
  }

  auto server_fd = LinuxFileDescriptor{socket(AF_INET, SOCK_STREAM, 0)};
  if (!server_fd.IsValid()) {
    return Error{Symbol::kLinuxEngineServerSocketFailed,
                 std::string{strerror(errno)}};
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
                 std::string{strerror(errno)}};
  }

  if (listen(server_fd.AsRaw(), SOMAXCONN) < 0) {
    return Error{Symbol::kLinuxEngineServerSocketListenFailed,
                 std::string{strerror(errno)}};
  }

  return LinuxEngine{std::move(epoll_fd), std::move(server_fd)};
}
