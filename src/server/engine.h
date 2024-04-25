#ifndef SERVER_ENGINE_H
#define SERVER_ENGINE_H

#include <memory>

#include "common.h"

class EngineImplDeleter final : private NonCopyable, Movable {
public:
  void operator()(void *impl_raw) const noexcept;
};

using EngineImplPtr = std::unique_ptr<void, EngineImplDeleter>;

class Engine final : private NonCopyable, Movable {
public:
  [[nodiscard]] auto Run() noexcept -> Result<Void>;

private:
  explicit Engine(EngineImplPtr &&impl) noexcept;

  EngineImplPtr impl_;

  friend class EngineBuilder;
};

class EngineBuilder final : private NonCopyable, NonMovable {
public:
  [[nodiscard]] auto Build(const uint16_t port) const noexcept
      -> Result<Engine>;
};

#endif // SERVER_ENGINE_H
