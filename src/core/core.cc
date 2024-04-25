#include "core.h"

#include <ostream>

auto operator<<(std::ostream &os, const Void &) -> std::ostream & {
  os << "Void";
  return os;
}
