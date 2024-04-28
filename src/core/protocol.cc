#include "protocol.h"

using namespace core;

auto
core::operator<<(std::ostream &os, const MessageKind kind) -> std::ostream & {
  os << static_cast<std::underlying_type_t<MessageKind>>(kind);
  return os;
}
