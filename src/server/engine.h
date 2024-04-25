#ifndef SERVER_ENGINE_H
#define SERVER_ENGINE_H

#include <memory>

#include "common.h"

class EngineImplDeleter final {
public:
  explicit EngineImplDeleter() noexcept = default;
  ~EngineImplDeleter() noexcept = default;
  CLASS_KIND_MOVABLE(EngineImplDeleter);

  void operator()(void *impl_raw) const noexcept;
};

using EngineImplPtr = std::unique_ptr<void, EngineImplDeleter>;

class Engine final {
public:
  class Builder final {
  public:
    explicit Builder() noexcept = default;
    ~Builder() noexcept = default;
    CLASS_KIND_PINNABLE(Builder);

    [[nodiscard]] auto Build(const uint16_t port) const noexcept
        -> Result<Engine>;
  };

  ~Engine() noexcept = default;
  CLASS_KIND_MOVABLE(Engine);

  [[nodiscard]] auto Run() noexcept -> Result<Void>;

private:
  explicit Engine(EngineImplPtr &&impl) noexcept;

  EngineImplPtr impl_;
};

#endif // SERVER_ENGINE_H
