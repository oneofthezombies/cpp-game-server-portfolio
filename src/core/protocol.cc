#include "protocol.h"

#include "tiny_json.h"

using namespace core;

auto
core::operator<<(std::ostream &os, const MessageKind kind) -> std::ostream & {
  switch (kind) {
    case MessageKind::kUndefined:
      os << "Undefined";
      break;
    case MessageKind::kClientRequest:
      os << "ClientRequest";
      break;
    case MessageKind::kClientRequestSuccess:
      os << "ClientRequestSuccess";
      break;
    case MessageKind::kClientRequestFailure:
      os << "ClientRequestFailure";
      break;
    case MessageKind::kClientEvent:
      os << "ClientEvent";
      break;
    case MessageKind::kServerRequest:
      os << "ServerRequest";
      break;
    case MessageKind::kServerRequestSuccess:
      os << "ServerRequestSuccess";
      break;
    case MessageKind::kServerRequestFailure:
      os << "ServerRequestFailure";
      break;
    case MessageKind::kServerEvent:
      os << "ServerEvent";
      break;
  }
  return os;
}

core::Message::Message(const MessageKind kind,
                       const Id id,
                       MessageBody &&body) noexcept
    : kind{kind}, id{id}, body{std::move(body)} {}

auto
core::Message::FromRaw(const std::string_view raw) -> std::optional<Message> {
  if (raw.size() < sizeof(MessageKind)) {
    return std::nullopt;
  }

  size_t offset{};
  const auto kind = *reinterpret_cast<const MessageKind *>(raw.data());
  offset += sizeof(MessageKind);

  const auto id = *reinterpret_cast<const Id *>(raw.data() + offset);
  offset += sizeof(Id);

  const auto payload = raw.substr(offset);
  auto json = TinyJson::Parse(payload);
  if (!json) {
    return std::nullopt;
  }

  return Message{kind, id, std::move(*json)};
}

auto
core::Message::BuildRaw(const MessageKind kind,
                        const Id id,
                        std::string &&tiny_json_str) noexcept -> std::string {
  std::string raw;
  raw.reserve(sizeof(MessageKind) + sizeof(Id) + tiny_json_str.size());

  raw.append(reinterpret_cast<const char *>(&kind), sizeof(MessageKind));
  raw.append(reinterpret_cast<const char *>(&id), sizeof(Id));
  raw.append(tiny_json_str);

  return raw;
}

auto
core::operator<<(std::ostream &os,
                 const Message &message) noexcept -> std::ostream & {
  os << "Message{";
  os << "kind=";
  os << message.kind;
  os << ", ";
  os << "id=";
  os << message.id;
  os << ", ";
  os << "body=";
  os << message.body;
  os << "}";
  return os;
}
