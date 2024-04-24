#include <cassert>
#include <cstring>
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
       tokenizer.Eat(), current = tokenizer.Current()) {
    const auto token = *current;

    if (token == "--help") {
      return Error{Symbol::kHelpRequested};
    }

    if (token == "--port") {
      const auto next = tokenizer.Next();
      if (!next.has_value()) {
        return Error{Symbol::kPortNotFound};
      }

      auto result = core::ParseNumberString<uint16_t>(*next);
      if (result.IsErr()) {
        return Error{Symbol::kPortParsingFailed,
                     std::make_error_code(result.Err()).message()};
      }

      options.port = result.Ok();
      tokenizer.Eat();
      continue;
    }

    return Error{Symbol::kUnknownArgument, std::string{token}};
  }
  return options;
}

auto main(int argc, char **argv) noexcept -> int {
  auto args_res = ParseArgs(core::ParseArgcArgv(argc, argv));
  if (args_res.IsErr()) {
    const auto &error = args_res.Err();
    if (error.code == Symbol::kHelpRequested) {
      std::cout << "Usage: server [--port <port>]";
    } else if (error.code == Symbol::kPortNotFound) {
      std::cout << "Error: port not found";
    } else if (error.code == Symbol::kPortParsingFailed) {
      std::cout << "Error: port parsing failed: " << error.message;
    } else if (error.code == Symbol::kUnknownArgument) {
      std::cout << "Error: unknown argument";
    } else {
      std::cout << "Error: " << static_cast<int>(error.code) << error.message;
    }

    std::cout << std::endl;
    return 1;
  }

  const auto &options = args_res.Ok();
  auto engine_res = EngineBuilder{}.Build(options.port);
  if (engine_res.IsErr()) {
    std::cout << "Error: engine creation failed: " << engine_res.Err().message
              << std::endl;
    return 1;
  }

  auto &engine = engine_res.Ok();
  if (auto res = engine.Run(); res.IsErr()) {
    std::cout << "Error: engine run failed: " << res.Err().message << std::endl;
    return 1;
  }
  return 0;
}
