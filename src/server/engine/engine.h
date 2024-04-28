#ifndef SERVER_ENGINE_ENGINE_H
#define SERVER_ENGINE_ENGINE_H

#include <memory>

#include "common.h"
#include "config.h"
#include "event_loop.h"

namespace engine {

class EngineImplRawDeleter final {
 public:
  explicit EngineImplRawDeleter() noexcept = default;
  ~EngineImplRawDeleter() noexcept = default;
  CLASS_KIND_MOVABLE(EngineImplRawDeleter);

  void
  operator()(void *impl_raw) const noexcept;
};

using EngineImplPtr = std::unique_ptr<void, EngineImplRawDeleter>;

class Engine final {
 public:
  class Builder final {
   public:
    explicit Builder() noexcept = default;
    ~Builder() noexcept = default;
    CLASS_KIND_PINNABLE(Builder);

    [[nodiscard]] auto
    Build(Config &&config) const noexcept -> Result<Engine>;
  };

  ~Engine() noexcept = default;
  CLASS_KIND_MOVABLE(Engine);

  [[nodiscard]] auto
  RegisterEventLoop(std::string &&name,
                    EventLoopHandlerPtr &&handler) noexcept -> Result<Void>;

  [[nodiscard]] auto
  Run() noexcept -> Result<Void>;

 private:
  explicit Engine(EngineImplPtr &&impl) noexcept;

  EngineImplPtr impl_;
};

}  // namespace engine

#endif  // SERVER_ENGINE_ENGINE_H
