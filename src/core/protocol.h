#ifndef CORE_PROTOCOL_H
#define CORE_PROTOCOL_H

#include <cstdint>
#include <optional>

#include "core.h"
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

using MessageBody = TinyJson;

struct Message final {
  using Id = uint64_t;

  MessageKind kind;
  Id id;
  MessageBody body;

  explicit Message(const MessageKind kind,
                   const Id id,
                   MessageBody &&body) noexcept;
  ~Message() noexcept = default;
  CLASS_KIND_MOVABLE(Message);

  [[nodiscard]] static auto
  FromRaw(const std::string_view raw) -> std::optional<Message>;

  [[nodiscard]] static auto
  BuildRaw(const MessageKind kind,
           const Id id,
           std::string &&tiny_json_str) noexcept -> std::string;
};

auto
operator<<(std::ostream &os, const Message &message) noexcept -> std::ostream &;

}  // namespace core

#endif  // CORE_PROTOCOL_H
