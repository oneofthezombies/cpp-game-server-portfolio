#include "common.h"

using namespace engine;

auto
engine::operator<<(std::ostream &os, const Symbol symbol) noexcept
    -> std::ostream & {
  os << "Symbol{";
  os << static_cast<i32>(symbol);
  os << "}";
  return os;
}
