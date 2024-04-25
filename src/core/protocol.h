#ifndef CORE_PROTOCOL_H
#define CORE_PROTOCOL_H

#include <optional>

#include "core.h"
#include "tiny_json.h"

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

struct Message final {
  using Id = uint64_t;

  MessageKind kind;
  Id id;
  TinyJson tiny_json;

  explicit Message(const MessageKind kind, const Id id,
                   TinyJson &&json) noexcept;
  ~Message() noexcept = default;
  CLASS_KIND_MOVABLE(Message);

  [[nodiscard]] static auto FromRaw(const std::string_view raw)
      -> std::optional<Message>;

  [[nodiscard]] static auto BuildRaw(const MessageKind kind, const Id id,
                                     std::string &&tiny_json_str) noexcept
      -> std::string;
};

auto operator<<(std::ostream &os, const Message &message) noexcept
    -> std::ostream &;

#endif // CORE_PROTOCOL_H
