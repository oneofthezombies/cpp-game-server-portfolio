#ifndef KERO_ENGINE_CONSTANTS_H
#define KERO_ENGINE_CONSTANTS_H

#include "kero/engine/component.h"

namespace kero {

struct ComponentKind {
  enum : Component::Kind {
    kUnspecified = 0,
    kSignal = 1,
    kActor = 2,
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

}  // namespace kero

#endif  // KERO_ENGINE_CONSTANTS_H
