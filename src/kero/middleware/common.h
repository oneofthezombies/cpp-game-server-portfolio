#ifndef KERO_MIDDLEWARE_COMMON_H
#define KERO_MIDDLEWARE_COMMON_H

#include "kero/engine/common.h"
#include "kero/engine/service_kind.h"

namespace kero {

using SocketId = u64;

enum : ServiceKindId {
  kServiceKindId_MiddlewareBegin = kServiceKindId_EngineEnd,

  kServiceKindId_IoEventLoop,
  kServiceKindId_TcpServer,
  kServiceKindId_Config,
  kServiceKindId_SocketRouter,

  kServiceKindId_MiddlewareEnd,
};

struct EventSocketOpen {
  static constexpr auto kEvent = "socket_open";
  static constexpr auto kSocketId = "socket_id";
};

struct EventSocketError {
  static constexpr auto kEvent = "socket_error";
  static constexpr auto kSocketId = "socket_id";
  static constexpr auto kErrorCode = "error_code";
  static constexpr auto kErrorDescription = "error_description";
};

struct EventSocketClose {
  static constexpr auto kEvent = "socket_close";
  static constexpr auto kSocketId = "socket_id";
};

struct EventSocketRead {
  static constexpr auto kEvent = "socket_read";
  static constexpr auto kSocketId = "socket_id";
};

struct EventSocketMove {
  static constexpr auto kEvent = "socket_move";
  static constexpr auto kSocketId = "socket_id";
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_COMMON_H
