#ifndef KERO_ENGINE_CONSTANTS_H
#define KERO_ENGINE_CONSTANTS_H

#include "kero/engine/service.h"

namespace kero {

static const Service::Kind kServiceKindSignal = {1, "signal"};
static const Service::Kind kServiceKindActor = {2, "actor"};
static const Service::Kind kServiceKindIoEventLoop = {3, "io_event_loop"};
static const Service::Kind kServiceKindTcpServer = {4, "tcp_server"};
static const Service::Kind kServiceKindConfig = {5, "config"};
static const Service::Kind kServiceKindSocketPool = {6, "socket_pool"};
static const Service::Kind kServiceKindSocketRouter = {7, "socket_router"};

struct EventShutdown {
  static constexpr auto kEvent = "shutdown";
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

#endif  // KERO_ENGINE_CONSTANTS_H
