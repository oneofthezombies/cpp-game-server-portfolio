#include <cassert>
#include <iostream>
#include <system_error>

#include "core/core.h"
#include "core/utils.h"

enum class ServerErrorCode : int32_t {
  kHelpRequested = 0,
  kPortNotFound,
  kPortParsingFailed,

  // Add more error codes before kCount
  kCount
};

using Error = core::Error<ServerErrorCode>;

template <typename T> using Result = core::Result<T, Error>;

struct ServerOptions final : private core::NonCopyable, core::Movable {
  uint16_t port;

  static constexpr uint16_t kUndefinedPort{0};
};

auto ParseArgs(core::Args &&args) noexcept -> Result<ServerOptions> {
  ServerOptions options;
  core::Lexer lexer{std::move(args)};
  for (auto current = lexer.Current(); current.has_value(); lexer.Eat()) {
    if (!current.has_value()) {
      break;
    }

    if (*current == "--help") {
      return Error{ServerErrorCode::kHelpRequested};
    }

    if (*current == "--port") {
      const auto next = lexer.Next();
      if (!next.has_value()) {
        return Error{ServerErrorCode::kPortNotFound};
      }

      auto result = core::ParseNumberString<uint16_t>(*next);
      if (result.IsErr()) {
        return Error{ServerErrorCode::kPortParsingFailed,
                     std::make_error_code(result.Err()).message()};
      }

      options.port = result.Ok();
      lexer.Eat();
    }
  }
  return options;
}

auto main(int argc, char **argv) noexcept -> int {
  auto args = core::ParseArgcArgv(argc, argv);
  auto result = ParseArgs(std::move(args));
  if (result.IsErr()) {
    const auto &error = result.Err();
    switch (error.code) {
    case ServerErrorCode::kHelpRequested:
      std::cout << "Usage: server [--port <port>]\n";
      break;
    case ServerErrorCode::kPortNotFound:
      std::cout << "Error: port not found\n";
      break;
    case ServerErrorCode::kPortParsingFailed:
      std::cout << "Error: port parsing failed: " << error.message << "\n";
      break;
    case ServerErrorCode::kCount:
      assert(false && "Do not use kCount as an error code");
      break;
    }

    std::cout << std::flush;
    return 1;
  }

  return 0;
}
