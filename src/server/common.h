#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H

#include <cstdint>

#include "core/core.h"
#include "core/utils.h"

/**
 * Server symbols start from 1,000,000
 */
enum class Symbol : int32_t {
  kBegin = 1'000'000,
  // Add symbols after kBegin

  kHelpRequested,
  kPortArgNotFound,
  kPortValueNotFound,
  kPortParsingFailed,
  kUnknownArgument,

  kLinuxEventLoopEpollCreate1Failed,
  kLinuxEventLoopEpollCtlAddFailed,
  kLinuxEventLoopEpollCtlDeleteFailed,
  kLinuxEventLoopEpollWaitFailed,

  kLinuxMainEventLoopServerSocketFailed,
  kLinuxMainEventLoopServerSocketBindFailed,
  kLinuxMainEventLoopServerSocketListenFailed,
  kLinuxMainEventLoopServerSocketAcceptFailed,
  kLinuxMainEventLoopUnexpectedFd,

  kLinuxLobbyEventLoopClientFdConversionFailed,

  kLinuxEngineClientSocketReadFailed,
  kLinuxEngineClientSocketWriteFailed,
  kLinuxEngineClientSocketClosed,
  kLinuxEngineClientSocketCloseFailed,

  kLinuxEngineSessionAlreadyExists,
  kLinuxEngineSessionNotFound,

  kLinuxEngineMessageParseFailed,

  kLinuxFileDescriptorGetStatusFailed,
  kLinuxFileDescriptorSetStatusFailed,
  kLinuxFileDescriptorCloseFailed,

  kLinuxSignalSetFailed,
  kLinuxSignalResetFailed,

  kThreadWorkerSessionAlreadyExists,

  // Add symbols before kEnd
  kEnd
};

auto operator<<(std::ostream &os, const Symbol symbol) noexcept
    -> std::ostream &;

using Error = core::Error<Symbol>;

template <typename T> using Result = core::Result<T, Error>;

using SB = core::StringBuilder;

#endif // SERVER_COMMON_H
