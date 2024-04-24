#ifndef SERVER_ENGINE_H
#define SERVER_ENGINE_H

#include <memory>

#include "common.h"

class EngineImplDeleter final : private core::NonCopyable, core::Movable {
public:
  void operator()(void *impl_raw) const noexcept;
};

using EngineImplPtr = std::unique_ptr<void, EngineImplDeleter>;

class Engine final : private core::NonCopyable, core::Movable {
public:
  [[nodiscard]] auto Run() noexcept -> ResultMany<core::Void>;

private:
  explicit Engine(EngineImplPtr &&impl) noexcept;

  EngineImplPtr impl_;

  friend class EngineBuilder;
};

class EngineBuilder final : private core::NonCopyable, core::NonMovable {
public:
  [[nodiscard]] auto Build(const uint16_t port) const noexcept
      -> Result<Engine>;
};

#endif // SERVER_ENGINE_H
