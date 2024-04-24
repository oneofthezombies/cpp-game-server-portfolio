#include "protocol.h"
#include "tiny_json.h"

using namespace core;

auto core::operator<<(std::ostream &os, const MessageKind kind)
    -> std::ostream & {
  switch (kind) {
  case MessageKind::kUndefined:
    os << "Undefined";
    break;
  case MessageKind::kRequest:
    os << "Request";
    break;
  case MessageKind::kRequestSuccess:
    os << "RequestSuccess";
    break;
  case MessageKind::kRequestFailure:
    os << "RequestFailure";
    break;
  case MessageKind::kRequestNoReply:
    os << "RequestNoReply";
    break;
  case MessageKind::kCommand:
    os << "Command";
    break;
  case MessageKind::kCommandSuccess:
    os << "CommandSuccess";
    break;
  case MessageKind::kCommandFailure:
    os << "CommandFailure";
    break;
  case MessageKind::kCommandNoReply:
    os << "CommandNoReply";
    break;
  }
  return os;
}

Message::Message(const MessageKind kind, const IdType id,
                 TinyJson &&json) noexcept
    : kind{kind}, id{id}, json{std::move(json)} {}

auto Message::FromRaw(const std::string_view raw) -> std::optional<Message> {
  if (raw.size() < sizeof(MessageKind)) {
    return std::nullopt;
  }

  size_t offset{};
  const auto kind = *reinterpret_cast<const MessageKind *>(raw.data());
  offset += sizeof(MessageKind);

  const auto id = *reinterpret_cast<const IdType *>(raw.data() + offset);
  offset += sizeof(IdType);

  const auto payload = raw.substr(offset);
  auto json = TinyJson::Parse(payload);
  if (!json) {
    return std::nullopt;
  }

  return Message{kind, id, std::move(*json)};
}

auto Message::BuildRaw(const MessageKind kind, const IdType id,
                       std::string &&tiny_json_str) noexcept -> std::string {
  std::string raw;
  raw.reserve(sizeof(MessageKind) + sizeof(IdType) + tiny_json_str.size());

  raw.append(reinterpret_cast<const char *>(&kind), sizeof(MessageKind));
  raw.append(reinterpret_cast<const char *>(&id), sizeof(IdType));
  raw.append(tiny_json_str);

  return raw;
}
