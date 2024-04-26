#ifndef SERVER_ENGINE_EVENT_LOOP_H
#define SERVER_ENGINE_EVENT_LOOP_H

#include <memory>

#include "core/core.h"

#include "common.h"
#include "event_loop_handler.h"
#include "server/engine/session_service.h"

namespace engine {

class EventLoopImplRawDeleter final {
public:
  explicit EventLoopImplRawDeleter() noexcept = default;
  ~EventLoopImplRawDeleter() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoopImplRawDeleter);

  auto operator()(void *impl_raw) const noexcept -> void;
};

using EventLoopImplPtr = std::unique_ptr<void, EventLoopImplRawDeleter>;

class EventLoop final {
public:
  class Builder final {
  public:
    explicit Builder() noexcept = default;
    ~Builder() noexcept = default;
    CLASS_KIND_PINNABLE(Builder);

    [[nodiscard]] auto Build(std::string &&name,
                             EventLoopHandlerPtr &&handler) const noexcept
        -> Result<EventLoop>;
  };

  ~EventLoop() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoop);

  [[nodiscard]] auto Init(const Config &config) noexcept -> Result<Void>;
  [[nodiscard]] auto Run() noexcept -> Result<Void>;
  [[nodiscard]] auto Name() const noexcept -> std::string_view;

  [[nodiscard]] auto Add(const SessionId session_id,
                         const uint32_t events) const noexcept -> Result<Void>;

private:
  explicit EventLoop(EventLoopImplPtr &&impl) noexcept;

  EventLoopImplPtr impl_;
};

} // namespace engine

#endif // SERVER_ENGINE_EVENT_LOOP_H
