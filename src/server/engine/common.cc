#include "common.h"

using namespace engine;

auto engine::operator<<(std::ostream &os,
                        const Symbol symbol) noexcept -> std::ostream & {
  os << "Symbol{";
  os << static_cast<int32_t>(symbol);
  os << "}";
  return os;
}
