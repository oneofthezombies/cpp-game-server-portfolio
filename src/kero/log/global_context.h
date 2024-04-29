#ifndef KERO_LOG_GLOBAL_CONTEXT_H
#define KERO_LOG_GLOBAL_CONTEXT_H

#include <deque>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "kero/core/mpsc_channel.h"
#include "kero/core/spsc_channel.h"
#include "kero/log/core.h"
#include "kero/log/runner_event.h"
#include "kero/log/transport.h"
#include "kero/log/utils.h"

namespace kero {

class GlobalContext final {
 public:
  class Builder {
   public:
    Builder() noexcept = default;
    ~Builder() noexcept = default;
    CLASS_KIND_PINNABLE(Builder);

    auto
    Build() const noexcept -> std::unique_ptr<GlobalContext>;
  };

  struct SharedState final {
    std::unordered_map<std::string, spsc::Rx<std::unique_ptr<kero::Log>>>
        log_rx_map{};
    std::deque<std::unique_ptr<kero::Log>> orphaned_logs{};
    std::vector<std::unique_ptr<Transport>> transports{};
    std::reference_wrapper<std::ostream> system_error_stream;

    SharedState(std::ostream& system_error_stream) noexcept;
    ~SharedState() noexcept = default;
    CLASS_KIND_PINNABLE(SharedState);
  };

  ~GlobalContext() noexcept = default;
  CLASS_KIND_PINNABLE(GlobalContext);

  auto
  UseSystemErrorStream(std::ostream& stream) noexcept -> void;

  auto
  LogSystemError(std::string&& message) noexcept -> void;

  [[nodiscard]] auto
  AddLogRx(const std::string& thread_id,
           spsc::Rx<std::unique_ptr<kero::Log>>&& log_rx) noexcept -> bool;

  [[nodiscard]] auto
  RemoveLogRx(const std::string& thread_id) noexcept -> bool;

  auto
  Shutdown(ShutdownConfig&& config) noexcept -> void;

  auto
  AddTransport(std::unique_ptr<Transport>&& transport) noexcept -> void;

  [[nodiscard]] auto
  TryPopLog() noexcept -> Option<std::unique_ptr<kero::Log>>;

  auto
  HandleLog(const kero::Log& log) const noexcept -> void;

 private:
  GlobalContext(mpsc::Tx<std::unique_ptr<RunnerEvent>>&& runner_event_tx,
                std::thread&& runner_thread) noexcept;

  NullStream null_stream_{};
  SharedState shared_state_;
  mutable std::mutex shared_state_mutex_{};
  mpsc::Tx<std::unique_ptr<RunnerEvent>> runner_event_tx_;
  std::thread runner_thread_;
};

auto
GetGlobalContext() -> GlobalContext&;

auto
RunOnThread(mpsc::Rx<std::unique_ptr<RunnerEvent>>&& runner_event_rx) -> void;

}  // namespace kero

#endif  // KERO_LOG_GLOBAL_CONTEXT_H
