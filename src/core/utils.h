#ifndef CPP_GAME_SERVER_PORTFOLIO_CORE_UTILS_H
#define CPP_GAME_SERVER_PORTFOLIO_CORE_UTILS_H

#include <string_view>
#include <vector>

namespace core {

auto ParseArgcArgv(int argc, char *argv[]) noexcept
    -> std::vector<std::string_view>;

} // namespace core

#endif // CPP_GAME_SERVER_PORTFOLIO_CORE_UTILS_H
