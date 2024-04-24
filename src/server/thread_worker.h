#ifndef SERVER_THREAD_WORKER_H
#define SERVER_THREAD_WORKER_H

#include <thread>
#include <unordered_set>

#include "core/core.h"

#include "common.h"
#include "session.h"

class ThreadWorker final : private core::NonCopyable, core::Movable {
public:
  explicit ThreadWorker(const uint64_t id) noexcept;

  auto Id() const noexcept -> uint64_t;

  auto Shutdown() noexcept -> Result<core::Void>;

  auto AddSession(Session &&session) noexcept -> Result<core::Void>;

private:
  std::unordered_set<Session> sessions_{};
  std::thread thread_{};
  uint64_t id_{};
};

#endif // SERVER_THREAD_WORKER_H
