#include "thread_worker.h"

ThreadWorker::ThreadWorker(const uint64_t id) noexcept : id_{id} {}

auto ThreadWorker::Id() const noexcept -> uint64_t { return id_; }

auto ThreadWorker::Shutdown() noexcept -> Result<core::Void> {
  // TODO: Implement this function.
  return core::Void{};
}

auto ThreadWorker::AddSession(Session &&session) noexcept
    -> Result<core::Void> {
  if (auto found = sessions_.find(session); found != sessions_.end()) {
    return Error{
        Symbol::kThreadWorkerSessionAlreadyExists,
        SB{}.Add("thread_worker_id", id_).Add("session", session).Build()};
  }

  sessions_.emplace(std::move(session));
  return core::Void{};
}
