#include "main_event_loop_handler.h"

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "core/tiny_json.h"
#include "core/utils_linux.h"

#include "config.h"
#include "event_loop.h"

using namespace engine;

auto engine::MainEventLoopHandler::OnInit(const Config &config,
                                          const EventLoop &event_loop) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  auto server_fd_raw = socket(AF_INET, SOCK_STREAM, 0);
  if (!FileDescriptorLinux::IsValid(server_fd_raw)) {
    return ResultT{Error{Symbol::kMainEventLoopHandlerServerSocketFailed,
                         core::TinyJson{}
                             .Set("linux_error", core::LinuxError::FromErrno())
                             .ToString()}};
  }

  auto server_fd = FileDescriptorLinux{server_fd_raw};
  if (auto result = server_fd.UpdateNonBlocking(); result.IsErr()) {
    return ResultT{Error{std::move(result.Err())}};
  }

  struct sockaddr_in server_addr {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(config.port);

  if (bind(server_fd.AsRaw(), (struct sockaddr *)&server_addr,
           sizeof(server_addr)) < 0) {
    return ResultT{Error{Symbol::kMainEventLoopHandlerServerSocketBindFailed,
                         core::TinyJson{}
                             .Set("linux_error", core::LinuxError::FromErrno())
                             .ToString()}};
  }

  if (listen(server_fd.AsRaw(), SOMAXCONN) < 0) {
    return ResultT{Error{Symbol::kMainEventLoopHandlerServerSocketListenFailed,
                         core::TinyJson{}
                             .Set("linux_error", core::LinuxError::FromErrno())
                             .ToString()}};
  }

  if (auto res = event_loop.Add(server_fd.AsRaw(), EPOLLIN); res.IsErr()) {
    return res;
  }

  server_fd_.reset(new FileDescriptorLinux{std::move(server_fd)});
  return ResultT{Void{}};
}
