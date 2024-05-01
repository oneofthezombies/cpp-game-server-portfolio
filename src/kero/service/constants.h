#ifndef KERO_SERVICE_CONSTANTS_H
#define KERO_SERVICE_CONSTANTS_H

#include "kero/engine/service.h"

namespace kero {

static const ServiceKind kServiceKindIoEventLoop = {3, "io_event_loop"};
static const ServiceKind kServiceKindTcpServer = {4, "tcp_server"};
static const ServiceKind kServiceKindConfig = {5, "config"};
static const ServiceKind kServiceKindSocketPool = {6, "socket_pool"};
static const ServiceKind kServiceKindSocketRouter = {7, "socket_router"};

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

#endif  // KERO_SERVICE_CONSTANTS_H
