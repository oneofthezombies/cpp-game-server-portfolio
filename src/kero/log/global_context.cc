#include "global_context.h"

#include <iostream>
#include <optional>

#include "kero/log/core.h"
#include "kero/log/runner_event.h"
#include "kero/log/transport.h"

using namespace kero;

auto
kero::GlobalContext::Builder::Build() const noexcept
    -> std::unique_ptr<GlobalContext> {
  auto [runner_event_tx, runner_event_rx] =
      mpsc::Channel<std::unique_ptr<RunnerEvent>>::Builder{}.Build();
  auto runner_thread = std::thread{RunOnThread, std::move(runner_event_rx)};
  return std::unique_ptr<GlobalContext>{
      new GlobalContext{std::move(runner_event_tx), std::move(runner_thread)}};
}

kero::GlobalContext::SharedState::SharedState(
    std::ostream& system_error_stream) noexcept
    : system_error_stream{system_error_stream} {}

kero::GlobalContext::GlobalContext(
    mpsc::Tx<std::unique_ptr<RunnerEvent>>&& runner_event_tx,
    std::thread&& runner_thread) noexcept
    : shared_state_{null_stream_},
      runner_event_tx_{std::move(runner_event_tx)},
      runner_thread_{std::move(runner_thread)} {}

auto
kero::GlobalContext::UseStreamForLoggingSystemError(
    std::ostream& stream) noexcept -> void {
  std::lock_guard<std::mutex> lock(shared_state_mutex_);
  shared_state_.system_error_stream = std::ref(stream);
}

auto
kero::GlobalContext::LogSystemError(std::string&& message) noexcept -> void {
  std::lock_guard<std::mutex> lock(shared_state_mutex_);
  shared_state_.system_error_stream.get() << message << std::endl;
}

auto
kero::GlobalContext::AddLogRx(
    const std::string& thread_id,
    spsc::Rx<std::unique_ptr<kero::Log>>&& log_rx) noexcept -> bool {
  std::lock_guard<std::mutex> lock(shared_state_mutex_);
  if (shared_state_.log_rx_map.find(thread_id) !=
      shared_state_.log_rx_map.end()) {
    return false;
  }

  shared_state_.log_rx_map.emplace(thread_id, std::move(log_rx));
  return true;
}

auto
kero::GlobalContext::RemoveLogRx(const std::string& thread_id) noexcept
    -> bool {
  std::lock_guard<std::mutex> lock(shared_state_mutex_);
  auto entry = shared_state_.log_rx_map.find(thread_id);
  if (entry == shared_state_.log_rx_map.end()) {
    return false;
  }

  while (auto log = entry->second.TryReceive()) {
    shared_state_.orphaned_logs.push_back(std::move(log.TakeUnwrap()));
  }

  shared_state_.log_rx_map.erase(thread_id);
  return true;
}

auto
kero::GlobalContext::Shutdown(ShutdownConfig&& config) noexcept -> void {
  runner_event_tx_.Send(
      std::make_unique<RunnerEvent>(runner_event::Shutdown{std::move(config)}));
  runner_thread_.join();
}

auto
kero::GlobalContext::AddTransport(
    std::unique_ptr<Transport>&& transport) noexcept -> void {
  std::lock_guard<std::mutex> lock(shared_state_mutex_);
  shared_state_.transports.push_back(std::move(transport));
}

auto
kero::GlobalContext::TryPopLog() noexcept
    -> Option<std::unique_ptr<kero::Log>> {
  std::lock_guard<std::mutex> lock(shared_state_mutex_);

  if (!shared_state_.orphaned_logs.empty()) {
    auto log = std::move(shared_state_.orphaned_logs.front());
    shared_state_.orphaned_logs.pop_front();
    return log;
  }

  for (const auto& [thread_id, log_rx] : shared_state_.log_rx_map) {
    auto log = log_rx.TryReceive();
    if (!log) {
      continue;
    }

    return log.TakeUnwrap();
  }

  return None;
}

auto
kero::GlobalContext::HandleLog(const kero::Log& log) const noexcept -> void {
  std::lock_guard<std::mutex> lock(shared_state_mutex_);

  for (auto& transport : shared_state_.transports) {
    if (log.level > transport->GetLevel()) {
      continue;
    }

    transport->OnLog(log);
  }
}

auto
kero::GetGlobalContext() -> GlobalContext& {
  static std::unique_ptr<GlobalContext> global_context{nullptr};
  static std::once_flag flag{};
  std::call_once(flag,
                 []() { global_context = GlobalContext::Builder{}.Build(); });
  return *global_context;
}

auto
kero::RunOnThread(mpsc::Rx<std::unique_ptr<RunnerEvent>>&& runner_event_rx)
    -> void {
  std::optional<std::chrono::steady_clock::time_point> shutdown_deadline{};

  while (true) {
    auto event_opt = runner_event_rx.TryReceive();
    auto log_opt = GetGlobalContext().TryPopLog();
    if (shutdown_deadline) {
      if (std::chrono::steady_clock::now() > *shutdown_deadline) {
        break;
      }

      if (!event_opt && !log_opt) {
        break;
      }
    }

    if (event_opt) {
      auto event = event_opt.TakeUnwrap();
      if (!event) {
        GetGlobalContext().LogSystemError("Internal: *event must not be null.");
      } else {
        std::visit(
            [&shutdown_deadline](auto&& event) {
              using T = std::decay_t<decltype(event)>;

              if constexpr (std::is_same_v<T, runner_event::Shutdown>) {
                if (shutdown_deadline) {
                  GetGlobalContext().LogSystemError(
                      "Shutdown already scheduled.");
                  return;
                }

                runner_event::Shutdown& shutdown = event;
                shutdown_deadline =
                    std::chrono::steady_clock::now() + shutdown.config.timeout;
              } else {
                static_assert(always_false_v<T>,
                              "every RunnerEvent must be handled.");
              }
            },
            *event);
      }
    }

    if (log_opt) {
      auto log = log_opt.TakeUnwrap();
      if (!log) {
        GetGlobalContext().LogSystemError("Internal: *log must not be null.");
      } else {
        GetGlobalContext().HandleLog(*log);
      }
    }
  }
}
