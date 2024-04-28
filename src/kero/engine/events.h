#ifndef KERO_ENGINE_EVENTS_H
#define KERO_ENGINE_EVENTS_H

namespace kero {

constexpr auto kMessageShutdown = "__shutdown";

constexpr auto kEventMailToSend = "mail_to_send";
constexpr auto kEventMailReceived = "mail_received";

}  // namespace kero

#endif  // KERO_ENGINE_EVENTS_H
