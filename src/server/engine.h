#ifndef SERVER_ENGINE_H
#define SERVER_ENGINE_H

#include "common.h"

class Engine final : private core::NonCopyable, core::Movable {
public:
  Engine(Engine &&) noexcept;
  ~Engine() noexcept;

  auto operator=(Engine &&) noexcept -> Engine &;

  [[nodiscard]] auto Run() noexcept -> Result<core::Void>;

private:
  explicit Engine(void *impl) noexcept;

  void *impl_{};

  friend class EngineBuilder;
};

class EngineBuilder final : private core::NonCopyable, core::NonMovable {
public:
  [[nodiscard]] auto Build(const uint16_t port) const noexcept
      -> Result<Engine>;
};

#endif // SERVER_ENGINE_H
