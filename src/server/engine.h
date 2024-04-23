#ifndef CPP_GAME_SERVER_PORTFOLIO_SERVER_ENGINE_H
#define CPP_GAME_SERVER_PORTFOLIO_SERVER_ENGINE_H

#include <memory>

#include "core/core.h"

struct EngineImpl;

enum class ServerErrorCode : int32_t {
  kHelpRequested = 0,
  kPortNotFound,
  kPortParsingFailed,
  kUnknownArgument,
  kEngineCreationFailed,
  kEngineEpollCreationFailed,

  // Add more error codes before kCount
  kCount
};

using Error = core::Error<ServerErrorCode>;

template <typename T> using Result = core::Result<T, Error>;

class Engine final : private core::NonCopyable, core::Movable {
public:
private:
  explicit Engine(std::unique_ptr<EngineImpl> &&impl) noexcept;

  std::unique_ptr<EngineImpl> impl_;

  friend class EngineFactory;
};

class EngineFactory final : private core::NonCopyable, core::NonMovable {
public:
  auto Create() const noexcept -> Result<Engine>;
};

#endif // CPP_GAME_SERVER_PORTFOLIO_SERVER_ENGINE_H
