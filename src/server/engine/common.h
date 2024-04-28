#ifndef SERVER_ENGINE_COMMON_H
#define SERVER_ENGINE_COMMON_H

#include <cstdint>

#include "core/core.h"

namespace engine {

/**
 * Server engine symbols start from 2,000,000
 */
enum Symbol : int32_t {
  kServerEngineBegin = 2'000'000,
  // Add symbols after kBegin

  kConfigPortUndefined,
  kConfigPrimaryEventLoopNameEmpty,

  kEngineEventLoopAlreadyExists,

  kMainEventLoopHandlerLinuxServerSocketFailed,
  kMainEventLoopHandlerLinuxServerSocketSetOptFailed,
  kMainEventLoopHandlerLinuxServerSocketBindFailed,
  kMainEventLoopHandlerLinuxServerSocketListenFailed,
  kMainEventLoopHandlerLinuxUnexpectedSocketId,
  kMainEventLoopHandlerLinuxServerSocketAcceptFailed,

  kSocketEventLoopHandlerSocketIdAlreadyExists,
  kSocketEventLoopHandlerSocketIdNotFound,
  kSocketEventLoopHandlerMailSocketIdNotFound,
  kSocketEventLoopHandlerMailKindNotFound,
  kSocketEventLoopHandlerMailHandlerAlreadyExists,
  kSocketEventLoopHandlerMailHandlerNotFound,
  kSocketEventLoopHandlerMailHandlerFailed,

  kMailBoxAlreadyExists,
  kMailBoxNotFound,
  kMailBoxNameEmpty,
  kMailBoxNameTooLong,
  kMailBoxNameAllReserved,

  kEventLoopMessageKindDuplicated,
  kEventLoopMessageIdDuplicated,

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

auto
operator<<(std::ostream &os, const Symbol symbol) noexcept -> std::ostream &;

using Void = core::Void;
using Error = core::Error;

template <typename T>
using Result = core::Result<T>;

}  // namespace engine

#endif  // SERVER_ENGINE_COMMON_H
