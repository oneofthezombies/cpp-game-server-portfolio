#include <iostream>
#include <memory>
#include <system_error>
#include <vector>

#include "core/core.h"
#include "core/utils.h"

#include "engine/engine.h"
#include "session_service.h"

// struct ServerOptions final {
//   uint16_t port{kUndefinedPort};

//   explicit ServerOptions() noexcept = default;
//   ~ServerOptions() noexcept = default;
//   CLASS_KIND_MOVABLE(ServerOptions);

//   static constexpr uint16_t kUndefinedPort{0};
// };

// auto ParseArgs(Args &&args) noexcept -> Result<ServerOptions> {
//   using ResultT = Result<ServerOptions>;

//   ServerOptions options;
//   Tokenizer tokenizer{std::move(args)};

//   // Skip the first argument which is the program name
//   tokenizer.Eat();

//   for (auto current = tokenizer.Current(); current.has_value();
//        tokenizer.Eat(), current = tokenizer.Current()) {
//     const auto token = *current;

//     if (token == "--help") {
//       return ResultT{Error{Symbol::kHelpRequested}};
//     }

//     if (token == "--port") {
//       const auto next = tokenizer.Next();
//       if (!next.has_value()) {
//         return ResultT{Error{Symbol::kPortValueNotFound}};
//       }

//       auto result = ParseNumberString<uint16_t>(*next);
//       if (result.IsErr()) {
//         return ResultT{Error{Symbol::kPortParsingFailed,
//                              std::make_error_code(result.Err()).message()}};
//       }

//       options.port = result.Ok();
//       tokenizer.Eat();
//       continue;
//     }

//     return ResultT{Error{Symbol::kUnknownArgument, std::string{token}}};
//   }

//   if (options.port == ServerOptions::kUndefinedPort) {
//     return ResultT{Error{Symbol::kPortArgNotFound}};
//   }

//   return ResultT{std::move(options)};
// }

auto main(int argc, char **argv) noexcept -> int {
  struct B {};
  class A : public engine::SessionService<B> {
  public:
    explicit A() : engine::SessionService<B>{"A"} {}
  };

  std::unique_ptr<A> root_service{std::make_unique<A>()};
  std::vector<std::unique_ptr<engine::SessionService<>>> services;
  services.push_back(std::move(root_service));

  // auto args_res = ParseArgs(ParseArgcArgv(argc, argv));
  // if (args_res.IsErr()) {
  //   const auto &error = args_res.Err();
  //   if (error.code == Symbol::kHelpRequested) {
  //     std::cout << "Usage: server [--port <port>]";
  //   } else if (error.code == Symbol::kPortArgNotFound) {
  //     std::cout << "Error: --port argument not found";
  //   } else if (error.code == Symbol::kPortValueNotFound) {
  //     std::cout << "Error: --port argument value not found";
  //   } else if (error.code == Symbol::kPortParsingFailed) {
  //     std::cout << "Error: port parsing failed: " << error.message;
  //   } else if (error.code == Symbol::kUnknownArgument) {
  //     std::cout << "Error: unknown argument";
  //   } else {
  //     std::cout << "Error: " << error;
  //   }

  //   std::cout << std::endl;
  //   return 1;
  // }

  // const auto &options = args_res.Ok();
  // auto engine_res = Engine::Builder{}.Build(options.port);
  // if (engine_res.IsErr()) {
  //   std::cout << engine_res.Err() << std::endl;
  //   return 1;
  // }

  // auto &engine = engine_res.Ok();
  // if (auto res = engine.Run(); res.IsErr()) {
  //   std::cout << res.Err() << std::endl;
  //   return 1;
  // }

  return 0;
}
