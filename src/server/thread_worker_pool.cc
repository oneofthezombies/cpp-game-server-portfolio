#include "thread_worker_pool.h"

ThreadWorkerPool::ThreadWorkerPool(const uint32_t size) noexcept {
  workers_.reserve(size);
  for (uint32_t i = 0; i < size; ++i) {
    workers_.emplace_back(i);
  }
}

auto ThreadWorkerPool::Shutdown() noexcept -> ResultMany<core::Void> {
  std::vector<Error> errors{};
  for (auto &worker : workers_) {
    if (auto result = worker.Shutdown(); result.IsErr()) {
      errors.emplace_back(std::move(result.Err()));
    }
  }

  if (!errors.empty()) {
    return errors;
  }

  return core::Void{};
}
