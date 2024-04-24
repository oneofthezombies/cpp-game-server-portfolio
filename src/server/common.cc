#include "common.h"

// TODO: Implement `operator<<` for `Symbol`
auto operator<<(std::ostream &os, const Symbol symbol) noexcept
    -> std::ostream & {
  os << "Symbol{";
  os << static_cast<int32_t>(symbol);
  os << "}";
  return os;
}
