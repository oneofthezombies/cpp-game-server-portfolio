#ifndef SERVER_EVENT_LOOP_H
#define SERVER_EVENT_LOOP_H

#include <memory>

#include "core/core.h"

#include "common.h"

namespace engine {

class EventLoopImplRawDeleter final {
public:
  explicit EventLoopImplRawDeleter() noexcept = default;
  ~EventLoopImplRawDeleter() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoopImplRawDeleter);

  auto operator()(void *impl_raw) const noexcept -> void;
};

using EventLoopImpl = std::unique_ptr<void, EventLoopImplRawDeleter>;

class EventLoop final {
public:
  class Builder final {
  public:
    explicit Builder() noexcept = default;
    ~Builder() noexcept = default;
    CLASS_KIND_PINNABLE(Builder);

    [[nodiscard]] auto Build() const noexcept -> Result<EventLoop>;
  };

  ~EventLoop() noexcept = default;
  CLASS_KIND_MOVABLE(EventLoop);

  [[nodiscard]] auto Run() noexcept -> Result<core::Void>;

private:
  explicit EventLoop(EventLoopImpl &&impl) noexcept;

  EventLoopImpl impl_;
};

} // namespace engine

#endif // SERVER_EVENT_LOOP_H