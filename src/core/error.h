#ifndef CPP_GAME_SERVER_PORTFOLIO_CORE_ERROR_H
#define CPP_GAME_SERVER_PORTFOLIO_CORE_ERROR_H

#include <cstdint>
#include <string>

namespace core {

enum class ErrorCode : int32_t { kCount };

struct Error {
  ErrorCode code;
  std::string message;

  Error(const ErrorCode code, std::string &&message) noexcept;
};

} // namespace core

#endif // CPP_GAME_SERVER_PORTFOLIO_CORE_ERROR_H