#ifndef CORE_PROTOCOL_H
#define CORE_PROTOCOL_H

#include <cstdint>
#include <ostream>

#include "tiny_json.h"

namespace core {

enum class MessageKind : uint8_t {
  // undefined message kind
  kUndefined = 0,

  // client to server, a success or failure is expected
  kClientRequest = 1,

  // server to client, after processing request with success
  kClientRequestSuccess = 2,

  // server to client, after processing request with failure
  kClientRequestFailure = 3,

  // client to server, no reply expected
  kClientEvent = 4,

  // server to client, a success or failure is expected
  kServerRequest = 5,

  // client to server, after processing command with success
  kServerRequestSuccess = 6,

  // client to server, after processing command with failure
  kServerRequestFailure = 7,

  // server to client, no reply expected
  kServerEvent = 8,
};

auto
operator<<(std::ostream &os, const MessageKind kind) -> std::ostream &;

using MessageId = uint64_t;
using Message = TinyJson;

}  // namespace core

#endif  // CORE_PROTOCOL_H
