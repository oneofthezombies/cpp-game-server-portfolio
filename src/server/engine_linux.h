#ifndef SERVER_ENGINE_LINUX_H
#define SERVER_ENGINE_LINUX_H

#include <unordered_map>

#include "file_descriptor_linux.h"
#include "thread_worker_pool.h"

class LinuxEngine final : private core::NonCopyable, core::Movable {
public:
  LinuxEngine(LinuxEngine &&) noexcept = default;
  ~LinuxEngine() noexcept = default;

  auto operator=(LinuxEngine &&) noexcept -> LinuxEngine & = default;

  [[nodiscard]] auto Run() noexcept -> ResultMany<core::Void>;

private:
  LinuxEngine(ThreadWorkerPool &&thread_worker_pool,
              LinuxFileDescriptor &&epoll_fd,
              LinuxFileDescriptor &&server_fd) noexcept;

  auto OnServerFdEvent() noexcept -> Result<core::Void>;
  auto OnClientFdEvent(const LinuxFileDescriptor::Raw client_fd) noexcept
      -> Result<core::Void>;

  auto DeleteConnectedSessionOrCloseFd(
      const LinuxFileDescriptor::Raw client_fd) noexcept -> void;

  std::unordered_map<Session::IdType, Session> connected_sessions_{};
  ThreadWorkerPool thread_worker_pool_;
  LinuxFileDescriptor epoll_fd_;
  LinuxFileDescriptor server_fd_;

  static constexpr size_t kMaxEvents = 64;

  friend class LinuxEngineBuilder;
};

class LinuxEngineBuilder final : private core::NonCopyable, core::NonMovable {
public:
  [[nodiscard]] auto Build(const uint16_t port,
                           ThreadWorkerPool &&thread_worker_pool) const noexcept
      -> Result<LinuxEngine>;
};

#endif // SERVER_ENGINE_LINUX_H
