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

  kMainEventLoopHandlerLinuxServerSocketFailed,
  kMainEventLoopHandlerLinuxServerSocketBindFailed,
  kMainEventLoopHandlerLinuxServerSocketListenFailed,
  kMainEventLoopHandlerLinuxUnexpectedSessionEvent,
  kMainEventLoopHandlerLinuxUnexpectedSessionId,
  kMainEventLoopHandlerLinuxServerSocketAcceptFailed,

  MainEventLoopHandlerBuilderPrimaryEventLoopNameEmpty,

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

  kFileDescriptorLinuxGetStatusFailed,
  kFileDescriptorLinuxSetStatusFailed,
  kFileDescriptorLinuxCloseFailed,
  kFileDescriptorLinuxParseSessionIdToFdFailed,

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

} // namespace engine

#endif // SERVER_ENGINE_COMMON_H
