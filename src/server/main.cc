#include <cassert>
#include <iostream>
#include <system_error>

#include "core/core.h"
#include "core/utils.h"

#include "engine.h"

struct ServerOptions final : private core::NonCopyable, core::Movable {
  uint16_t port;

  static constexpr uint16_t kUndefinedPort{0};
};

auto ParseArgs(core::Args &&args) noexcept -> Result<ServerOptions> {
  ServerOptions options;
  core::Tokenizer tokenizer{std::move(args)};

  // Skip the first argument which is the program name
  tokenizer.Eat();

  for (auto current = tokenizer.Current(); current.has_value();
       tokenizer.Eat()) {
    const auto token = *current;

    if (token == "--help") {
      return Error{ServerErrorCode::kHelpRequested};
    }

    if (token == "--port") {
      const auto next = tokenizer.Next();
      if (!next.has_value()) {
        return Error{ServerErrorCode::kPortNotFound};
      }

      auto result = core::ParseNumberString<uint16_t>(*next);
      if (result.IsErr()) {
        return Error{ServerErrorCode::kPortParsingFailed,
                     std::make_error_code(result.Err()).message()};
      }

      options.port = result.Ok();
      tokenizer.Eat();
    }

    return Error{ServerErrorCode::kUnknownArgument, std::string{token}};
  }
  return options;
}

auto main(int argc, char **argv) noexcept -> int {
  auto result = ParseArgs(core::ParseArgcArgv(argc, argv));
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
    case ServerErrorCode::kUnknownArgument:
      std::cout << "Error: unknown argument\n";
      break;
    case ServerErrorCode::kEngineCreationFailed:
      std::cout << "Error: engine creation failed\n";
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
