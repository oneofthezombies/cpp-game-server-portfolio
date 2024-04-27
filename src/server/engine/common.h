#ifndef SERVER_ENGINE_COMMON_H
#define SERVER_ENGINE_COMMON_H

#include <cstdint>

#include "core/core.h"

namespace engine {

/**
 * Server engine symbols start from 1,000,000
 */
enum Symbol : int32_t {
  kServerEngineBegin = 1'000'000,
  // Add symbols after kBegin

  kConfigPortUndefined,
  kConfigPrimaryEventLoopNameEmpty,

  kEngineEventLoopAlreadyExists,

  kMainEventLoopHandlerLinuxServerSocketFailed,
  kMainEventLoopHandlerLinuxServerSocketBindFailed,
  kMainEventLoopHandlerLinuxServerSocketListenFailed,
  kMainEventLoopHandlerLinuxUnexpectedSocketId,
  kMainEventLoopHandlerLinuxServerSocketAcceptFailed,

  kSocketEventLoopHandlerParseSocketIdFailed,
  kSocketEventLoopHandlerSocketIdAlreadyExists,
  kSocketEventLoopHandlerSocketIdNotFound,

  kMailBoxAlreadyExists,
  kMailBoxNotFound,
  kMailBoxNameEmpty,
  kMailBoxNameTooLong,
  kMailBoxNameAllReserved,

  kEventLoopLinuxEpollCreate1Failed,
  kEventLoopLinuxEpollCtlAddFailed,
  kEventLoopLinuxEpollCtlDeleteFailed,
  kEventLoopLinuxEpollWaitFailed,
  kEventLoopLinuxWriteFailed,
  kEventLoopLinuxWriteClosed,
  kEventLoopLinuxGetSocketOptionFailed,
  kEventLoopLinuxSocketErrorZero,

  kFileDescriptorLinuxGetStatusFailed,
  kFileDescriptorLinuxSetStatusFailed,
  kFileDescriptorLinuxCloseFailed,
  kFileDescriptorLinuxParseFdToSocketIdFailed,
  kFileDescriptorLinuxParseSocketIdToFdFailed,

  kLinuxSignalSetFailed,
  kLinuxSignalResetFailed,

  // Add symbols before kEnd
  kServerEngineEnd
};

auto operator<<(std::ostream &os, const Symbol symbol) noexcept
    -> std::ostream &;

using Void = core::Void;
using Error = core::Error<int32_t>;

template <typename T> using Result = core::Result<T, Error>;

} // namespace engine

#endif // SERVER_ENGINE_COMMON_H
