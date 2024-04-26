#ifndef SERVER_ENGINE_COMMON_H
#define SERVER_ENGINE_COMMON_H

#include <cstdint>

#include "core/core.h"

namespace engine {

/**
 * Server symbols start from 1,000,000
 */
enum class Symbol : int32_t {
  kBegin = 1'000'000,
  // Add symbols after kBegin

  kConfigPortUndefined,
  kConfigPrimarySessionServiceNotFound,

  kEngineEventLoopAlreadyExists,

  kMainEventLoopHandlerServerSocketFailed,
  kMainEventLoopHandlerServerSocketBindFailed,
  kMainEventLoopHandlerServerSocketListenFailed,

  kHelpRequested,
  kPortArgNotFound,
  kPortValueNotFound,
  kPortParsingFailed,
  kUnknownArgument,

  kMailBoxAlreadyExists,
  kMailBoxNotFound,
  kMailBoxNameEmpty,
  kMailBoxNameTooLong,
  kMailBoxNameAll,

  kEventLoopLinuxEpollCreate1Failed,
  kEventLoopLinuxEpollCtlAddFailed,
  kEventLoopLinuxEpollCtlDeleteFailed,
  kEventLoopLinuxEpollWaitFailed,
  kEventLoopLinuxWriteFailed,
  kEventLoopLinuxWriteClosed,
  kEventLoopLinuxParseSessionIdToFdFailed,

  kFileDescriptorLinuxGetStatusFailed,
  kFileDescriptorLinuxSetStatusFailed,
  kFileDescriptorLinuxCloseFailed,

  kLinuxSignalSetFailed,
  kLinuxSignalResetFailed,

  // Add symbols before kEnd
  kEnd
};

auto operator<<(std::ostream &os, const Symbol symbol) noexcept
    -> std::ostream &;

using Error = core::Error<Symbol>;

template <typename T> using Result = core::Result<T, Error>;

using Void = core::Void;

using SessionId = uint64_t;

} // namespace engine

#endif // SERVER_ENGINE_COMMON_H
