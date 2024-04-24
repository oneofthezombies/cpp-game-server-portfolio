#ifndef CORE_PROTOCOL_H
#define CORE_PROTOCOL_H

#include <optional>

#include "core.h"
#include "tiny_json.h"

namespace core {

enum class MessageKind : uint8_t {
  // undefined message kind
  kUndefined = 0,

  // client to server, a RequestSccess or RequestFailure is expected
  kRequest = 1,

  // server to client, after processing request with success
  kRequestSuccess = 2,

  // server to client, after processing request with failure
  kRequestFailure = 3,

  // client to server, no reply expected
  kRequestNoReply = 4,

  // server to client, a CommandSuccess or CommandFailure is expected
  kCommand = 5,

  // client to server, after processing command with success
  kCommandSuccess = 6,

  // client to server, after processing command with failure
  kCommandFailure = 7,

  // server to client, no reply expected
  kCommandNoReply = 8,
};

auto operator<<(std::ostream &os, const MessageKind kind) -> std::ostream &;

struct Message final : private NonCopyable, Movable {
  using IdType = uint64_t;

  MessageKind kind;
  IdType id;
  TinyJson json;

  explicit Message(const MessageKind kind, const IdType id,
                   TinyJson &&json) noexcept;

  [[nodiscard]] static auto FromRaw(const std::string_view raw)
      -> std::optional<Message>;

  [[nodiscard]] static auto BuildRaw(const MessageKind kind, const IdType id,
                                     std::string &&tiny_json_str) noexcept
      -> std::string;
};

} // namespace core

#endif // CORE_PROTOCOL_H
