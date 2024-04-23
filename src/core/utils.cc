#include "utils.h"

using namespace ootz;

auto ootz::parse_args(int argc, char **argv) noexcept
    -> std::vector<std::string_view> {
  return std::vector<std::string_view>{argv, argv + argc};
}
