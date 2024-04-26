#include "core.h"

#include <ostream>

using namespace core;

auto core::operator<<(std::ostream &os, const Void &) -> std::ostream & {
  os << "Void";
  return os;
}
