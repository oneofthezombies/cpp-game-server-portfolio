#ifndef KERO_ENGINE_CONSTANTS_H
#define KERO_ENGINE_CONSTANTS_H

#include "kero/engine/service.h"

namespace kero {

struct ServiceKind {
  enum : Service::Kind {
    kUnspecified = 0,
    kSignal = 1,
    kActor = 2,
    kIoEventLoop = 3,
    kTcpServer = 4,
    kConfig = 5,
    kSocket = 6,
  };
};

constexpr auto kMessageShutdown = "shutdown";

struct EventMailToSend {
  static constexpr auto kEvent = "mail_to_send";
  static constexpr auto kFrom = "from";
  static constexpr auto kTo = "to";
};

struct EventMailReceived {
  static constexpr auto kEvent = "mail_received";
  static constexpr auto kFrom = "from";
  static constexpr auto kTo = "to";
  static constexpr auto kBody = "body";
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

}  // namespace kero

#endif  // KERO_ENGINE_CONSTANTS_H
