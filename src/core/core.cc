#include "core.h"

#include <ostream>

using namespace core;

auto core::operator<<(std::ostream &os, const Void &) -> std::ostream & {
  os << "Void";
  return os;
}

auto core::operator<<(std::ostream &os,
                      const ErrorDetails &details) -> std::ostream & {
  os << "Details{";
  for (auto it = details.begin(); it != details.end(); ++it) {
    os << it->first;
    os << "=";
    os << it->second;
    if (std::next(it) != details.end()) {
      os << ", ";
    }
  }
  os << "}";
  return os;
}