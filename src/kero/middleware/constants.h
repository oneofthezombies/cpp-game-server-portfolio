#ifndef KERO_MIDDLEWARE_CONSTANTS_H
#define KERO_MIDDLEWARE_CONSTANTS_H

#include "kero/engine/service_kind.h"

namespace kero {

enum MiddlewareServiceKindId : ServiceKindId {
  kServiceKindIdIoEventLoop = 3,
  kServiceKindIdTcpServer = 4,
  kServiceKindIdConfig = 5,
  kServiceKindIdSocketPool = 6,
  kServiceKindIdSocketRouter = 7,
};

struct EventSocketError {
  static constexpr auto kEvent = "socket_error";
  static constexpr auto kFd = "fd";
  static constexpr auto kErrorCode = "error_code";
  static constexpr auto kErrorDescription = "error_description";
};

struct EventSocketClose {
  static constexpr auto kEvent = "socket_close";
  static constexpr auto kFd = "fd";
};

struct EventSocketRead {
  static constexpr auto kEvent = "socket_read";
  static constexpr auto kFd = "fd";
};

struct EventSocketOpen {
  static constexpr auto kEvent = "socket_open";
  static constexpr auto kFd = "fd";
};

struct EventSocketRegister {
  static constexpr auto kEvent = "socket_register";
  static constexpr auto kFd = "fd";
};

struct EventSocketUnregister {
  static constexpr auto kEvent = "socket_unregister";
  static constexpr auto kFd = "fd";
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_CONSTANTS_H
