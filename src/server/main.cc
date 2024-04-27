#include "core/tiny_json.h"
#include "core/utils.h"

#include "engine/config.h"
#include "engine/engine.h"

enum Symbol : int32_t {
  kHelpRequested = 0,
  kPortArgNotFound,
  kPortValueNotFound,
  kPortParsingFailed,
  kUnknownArgument,
};

auto operator<<(std::ostream &os, const Symbol &symbol) -> std::ostream & {
  os << "Symbol{";
  os << static_cast<int32_t>(symbol);
  os << "}";
  return os;
}

using Error = core::Error<Symbol>;

template <typename T> using Result = core::Result<T, Error>;

auto ParseArgs(core::Args &&args) noexcept -> Result<engine::Config>;

auto main(int argc, char **argv) noexcept -> int {
  auto config_res = ParseArgs(core::ParseArgcArgv(argc, argv));
  if (config_res.IsErr()) {
    const auto &error = config_res.Err();
    switch (error.code) {
    case Symbol::kHelpRequested:
      // noop
      break;
    case Symbol::kPortArgNotFound:
      core::TinyJson{}
          .Set("message", "port argument not found")
          .Set("error", error)
          .LogLn();
      break;
    case Symbol::kPortValueNotFound:
      core::TinyJson{}
          .Set("message", "port value not found")
          .Set("error", error)
          .LogLn();
      break;
    case Symbol::kPortParsingFailed:
      core::TinyJson{}
          .Set("message", "port parsing failed")
          .Set("error", error)
          .LogLn();
      break;
    case Symbol::kUnknownArgument:
      core::TinyJson{}
          .Set("message", "unknown argument")
          .Set("error", error)
          .LogLn();
      break;
    }

    core::TinyJson{}.Set("usage", "server [--port <port>]").LogLn();
    return 1;
  }

  auto config = std::move(config_res.Ok());
  config.primary_event_loop_name = "lobby";
  if (auto res = config.Validate(); res.IsErr()) {
    core::TinyJson{}
        .Set("message", "config validation failed")
        .Set("error", res.Err())
        .LogLn();
    return 1;
  }

  auto engine_res = engine::Engine::Builder{}.Build(std::move(config));
  if (engine_res.IsErr()) {
    core::TinyJson{}
        .Set("message", "engine build failed")
        .Set("error", engine_res.Err())
        .LogLn();
    return 1;
  }

  auto engine = std::move(engine_res.Ok());
  if (auto res = engine.Run(); res.IsErr()) {
    std::cout << res.Err() << std::endl;
    return 1;
  }

  return 0;
}

auto ParseArgs(core::Args &&args) noexcept -> Result<engine::Config> {
  using ResultT = Result<engine::Config>;

  engine::Config config{};
  core::Tokenizer tokenizer{std::move(args)};

  // Skip the first argument which is the program name
  tokenizer.Eat();

  for (auto current = tokenizer.Current(); current.has_value();
       tokenizer.Eat(), current = tokenizer.Current()) {
    const auto token = *current;

    if (token == "--help") {
      return ResultT{Error{Symbol::kHelpRequested}};
    }

    if (token == "--port") {
      const auto next = tokenizer.Next();
      if (!next.has_value()) {
        return ResultT{Error{Symbol::kPortValueNotFound}};
      }

      auto result = core::ParseNumberString<uint16_t>(*next);
      if (result.IsErr()) {
        return ResultT{
            Error{Symbol::kPortParsingFailed,
                  core::TinyJson{}.Set("error", result.Err()).IntoMap()}};
      }

      config.port = result.Ok();
      tokenizer.Eat();
      continue;
    }

    return ResultT{Error{Symbol::kUnknownArgument,
                         core::TinyJson{}.Set("token", token).IntoMap()}};
  }

  if (config.port == engine::Config::kUndefinedPort) {
    return ResultT{Error{Symbol::kPortArgNotFound}};
  }

  return ResultT{std::move(config)};
}
