#ifndef SERVER_ENGINE_ENGINE_H
#define SERVER_ENGINE_ENGINE_H

#include <memory>

#include "common.h"
#include "options.h"
#include "session_service.h"

namespace engine {

class EngineImplRawDeleter final {
public:
  explicit EngineImplRawDeleter() noexcept = default;
  ~EngineImplRawDeleter() noexcept = default;
  CLASS_KIND_MOVABLE(EngineImplRawDeleter);

  void operator()(void *impl_raw) const noexcept;
};

using EngineImpl = std::unique_ptr<void, EngineImplRawDeleter>;

class Engine final {
public:
  class Builder final {
  public:
    explicit Builder() noexcept = default;
    ~Builder() noexcept = default;
    CLASS_KIND_PINNABLE(Builder);

    [[nodiscard]] auto Build(const Options &options) const noexcept
        -> Result<Engine>;
  };

  ~Engine() noexcept = default;
  CLASS_KIND_MOVABLE(Engine);

  [[nodiscard]] auto AddSessionService(
      const std::string_view name,
      std::unique_ptr<SessionService<>> &&session_service) noexcept
      -> Result<Void>;

  [[nodiscard]] auto Run() noexcept -> Result<Void>;

private:
  explicit Engine(EngineImpl &&impl) noexcept;

  EngineImpl impl_;
};

} // namespace engine

#endif // SERVER_ENGINE_ENGINE_H
