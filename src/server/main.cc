#include <iostream>

#include "core/utils.h"

struct ServerParam {
  int32_t port{-1};

  static auto FromArgs(std::vector<std::string_view> &&args) noexcept
      -> Result<ServerParam, std::string>{

      };

  auto main(int argc, char **argv) noexcept -> int {
    auto args = ootz::ParseArgcArgv(argc, argv);
    const auto param = ServerParam::FromArgs(std::move(args));
    return 0;
  }
