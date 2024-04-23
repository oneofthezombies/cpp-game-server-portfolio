#include "utils.h"

using namespace core;

auto core::ParseArgcArgv(int argc, char **argv) noexcept
    -> std::vector<std::string_view> {
  return std::vector<std::string_view>{argv, argv + argc};
}
