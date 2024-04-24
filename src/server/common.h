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
  kPortNotFound,
  kPortParsingFailed,
  kUnknownArgument,
  kLinuxEngineEpollCreate1Failed,
  kLinuxEngineEpollCtlAddServerFailed,
  kLinuxEngineEpollCtlAddClientFailed,
  kLinuxEngineEpollWaitFailed,
  kLinuxEngineServerSocketFailed,
  kLinuxEngineServerSocketBindFailed,
  kLinuxEngineServerSocketListenFailed,
  kLinuxEngineServerSocketAcceptFailed,
  kLinuxEngineClientSocketReadFailed,
  kLinuxFileDescriptorGetStatusFailed,
  kLinuxFileDescriptorSetStatusFailed,
  kLinuxFileDescriptorCloseFailed,
  kLinuxSignalSetFailed,
  kLinuxSignalResetFailed,

  // Add symbols before kEnd
  kEnd
};

using Error = core::Error<Symbol>;

template <typename T> using Result = core::Result<T, Error>;

using SB = core::StringBuilder;

#endif // SERVER_COMMON_H
