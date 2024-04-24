#ifndef SERVER_THREAD_WORKER_POOL_H
#define SERVER_THREAD_WORKER_POOL_H

#include <vector>

#include "thread_worker.h"

class ThreadWorkerPool final : private core::NonCopyable, core::Movable {
public:
  explicit ThreadWorkerPool(const uint32_t size) noexcept;
  ThreadWorkerPool(ThreadWorkerPool &&) noexcept = default;
  ~ThreadWorkerPool() noexcept = default;

  auto operator=(ThreadWorkerPool &&) noexcept -> ThreadWorkerPool & = default;

  auto Shutdown() noexcept -> ResultMany<core::Void>;

private:
  std::vector<ThreadWorker> workers_;
};

#endif // SERVER_THREAD_WORKER_POOL_H
