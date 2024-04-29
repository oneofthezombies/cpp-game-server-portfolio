// #include "core/tiny_json.h"
// #include "core/utils.h"
// #include "engine/config.h"
// #include "engine/engine.h"
// #include "server/contents/battle.h"
// #include "server/contents/lobby.h"
// #include "server/engine/event_loop.h"

// enum Symbol : int32_t {
//   kHelpRequested = 0,
//   kPortArgNotFound,
//   kPortValueNotFound,
//   kPortParsingFailed,
//   kUnknownArgument,
// };

// auto
// operator<<(std::ostream &os, const Symbol &symbol) -> std::ostream & {
//   os << "Symbol{";
//   os << static_cast<int32_t>(symbol);
//   os << "}";
//   return os;
// }

// using Error = core::Error;

// template <typename T>
// using Result = core::Result<T>;

// auto
// ParseArgs(core::Args &&args) noexcept -> Result<engine::Config>;

// auto
// main(int argc, char **argv) noexcept -> int {
//   auto config_res = ParseArgs(core::ParseArgcArgv(argc, argv));
//   if (config_res.IsErr()) {
//     const auto &error = config_res.Err();
//     switch (error.code) {
//       case kHelpRequested:
//         // noop
//         break;
//       case kPortArgNotFound:
//         core::TinyJson{}
//             .Set("message", "port argument not found")
//             .Set("error", error)
//             .LogLn();
//         break;
//       case kPortValueNotFound:
//         core::TinyJson{}
//             .Set("message", "port value not found")
//             .Set("error", error)
//             .LogLn();
//         break;
//       case kPortParsingFailed:
//         core::TinyJson{}
//             .Set("message", "port parsing failed")
//             .Set("error", error)
//             .LogLn();
//         break;
//       case kUnknownArgument:
//         core::TinyJson{}
//             .Set("message", "unknown argument")
//             .Set("error", error)
//             .LogLn();
//         break;
//     }

//     core::TinyJson{}.Set("usage", "server [--port <port>]").LogLn();
//     return 1;
//   }

//   auto config = std::move(config_res.Ok());
//   config.primary_event_loop_name = "lobby";
//   if (auto res = config.Validate(); res.IsErr()) {
//     core::TinyJson{}
//         .Set("message", "config validation failed")
//         .Set("error", res.Err())
//         .LogLn();
//     return 1;
//   }

//   auto engine_res = engine::Engine::Builder{}.Build(std::move(config));
//   if (engine_res.IsErr()) {
//     core::TinyJson{}
//         .Set("message", "engine build failed")
//         .Set("error", engine_res.Err())
//         .LogLn();
//     return 1;
//   }

//   std::vector<std::pair<std::string, engine::EventLoopHandlerPtr>>
//   event_loops; event_loops.emplace_back("lobby",
//   std::make_unique<contents::Lobby>()); event_loops.emplace_back("battle",
//   std::make_unique<contents::Battle>());

//   auto engine = std::move(engine_res.Ok());
//   for (auto &[name, handler] : event_loops) {
//     if (auto res =
//             engine.RegisterEventLoop(std::string{name}, std::move(handler));
//         res.IsErr()) {
//       core::TinyJson{}
//           .Set("message", "register event loop handler failed")
//           .Set("error", res.Err())
//           .LogLn();
//       return 1;
//     }
//   }

//   if (auto res = engine.Run(); res.IsErr()) {
//     std::cout << res.Err() << std::endl;
//     return 1;
//   }

//   return 0;
// }

// auto
// ParseArgs(core::Args &&args) noexcept -> Result<engine::Config> {
//   using ResultT = Result<engine::Config>;

//   engine::Config config{};
//   core::Tokenizer tokenizer{std::move(args)};

//   // Skip the first argument which is the program name
//   tokenizer.Eat();

//   for (auto current = tokenizer.Current(); current.has_value();
//        tokenizer.Eat(), current = tokenizer.Current()) {
//     const auto token = *current;

//     if (token == "--help") {
//       return ResultT{Error::From(kHelpRequested)};
//     }

//     if (token == "--port") {
//       const auto next = tokenizer.Next();
//       if (!next.has_value()) {
//         return ResultT{Error::From(kPortValueNotFound)};
//       }

//       auto result = core::ParseNumberString<uint16_t>(*next);
//       if (result.IsErr()) {
//         return ResultT{
//             Error::From(kPortParsingFailed,
//                         core::TinyJson{}.Set("error",
//                         result.Err()).IntoMap())};
//       }

//       config.port = result.Ok();
//       tokenizer.Eat();
//       continue;
//     }

//     return ResultT{Error::From(kUnknownArgument,
//                                core::TinyJson{}.Set("token",
//                                token).IntoMap())};
//   }

//   if (config.port == engine::Config::kUndefinedPort) {
//     return ResultT{Error::From(kPortArgNotFound)};
//   }

//   return ResultT{std::move(config)};
// }

#include <memory>

#include "kero/core/utils.h"
#include "kero/engine/actor_system.h"
#include "kero/engine/agent.h"
#include "kero/engine/config_service.h"
#include "kero/engine/io_event_loop_service.h"
#include "kero/engine/signal_service.h"
#include "kero/engine/socket_pool_service.h"
#include "kero/engine/tcp_server_service.h"
#include "kero/log/center.h"
#include "kero/log/core.h"
#include "kero/log/log_builder.h"
#include "kero/log/transport.h"

auto
main(int argc, char** argv) -> int {
  kero::Center{}.UseStreamForLoggingSystemError();
  auto transport = std::make_unique<kero::ConsolePlainTextTransport>();
  transport->SetLevel(kero::Level::kDebug);
  kero::Center{}.AddTransport(std::move(transport));
  kero::Defer defer_log_system{[] { kero::Center{}.Shutdown(); }};

  const auto actor_system = kero::ActorSystem::Builder{}.Build();
  if (auto res = actor_system->Start(); res.IsErr()) {
    kero::log::Error("Actor system start failed")
        .Data("error", res.TakeErr())
        .Log();
    return 1;
  }

  kero::Defer defer_actor_system{[actor_system] {
    if (!actor_system->Stop()) {
      kero::log::Error("Actor system stop failed").Log();
    }
  }};

  auto main_agent = kero::Agent{};

  {
    auto actor_res = actor_system->CreateActorService("main");
    if (actor_res.IsErr()) {
      kero::log::Error("Actor system failed to create actor service")
          .Data("error", actor_res.TakeErr())
          .Log();
      return 1;
    }

    if (auto res = main_agent.AddService(actor_res.TakeOk()); !res) {
      kero::log::Error("Agent failed to add actor service").Log();
      return 1;
    }
  }

  {
    auto config_res = kero::ConfigService::FromArgs(argc, argv);
    if (config_res.IsErr()) {
      kero::log::Error("Config service failed to parse args")
          .Data("error", config_res.TakeErr())
          .Log();
      return 1;
    }

    if (auto res = main_agent.AddService(config_res.TakeOk()); !res) {
      kero::log::Error("Agent failed to add config service").Log();
      return 1;
    }
  }

  if (auto res = main_agent.AddService(std::make_unique<kero::SignalService>());
      !res) {
    kero::log::Error("Agent failed to add signal service").Log();
    return 1;
  }

  if (!main_agent.AddService(std::make_unique<kero::TcpServerService>())) {
    kero::log::Error("Agent failed to add tcp server service").Log();
    return 1;
  }

  if (!main_agent.AddService(std::make_unique<kero::IoEventLoopService>())) {
    kero::log::Error("Agent failed to add io event loop service").Log();
    return 1;
  }

  kero::Agent lobby_agent{};
  {
    auto lobby_res = actor_system->CreateActorService("lobby");
    if (lobby_res.IsErr()) {
      kero::log::Error("Actor system failed to create lobby actor service")
          .Data("error", lobby_res.TakeErr())
          .Log();
      return 1;
    }

    if (!lobby_agent.AddService(std::make_unique<kero::IoEventLoopService>())) {
      kero::log::Error("Agent failed to add io event loop service").Log();
      return 1;
    }

    if (!lobby_agent.AddService(std::make_unique<kero::SocketPoolService>())) {
      kero::log::Error("Agent failed to add socket pool service").Log();
      return 1;
    }
  }

  kero::Agent battle_agent{};
  {
    auto battle_res = actor_system->CreateActorService("battle");
    if (battle_res.IsErr()) {
      kero::log::Error("Actor system failed to create battle actor service")
          .Data("error", battle_res.TakeErr())
          .Log();
      return 1;
    }

    if (!battle_agent.AddService(
            std::make_unique<kero::IoEventLoopService>())) {
      kero::log::Error("Agent failed to add io event loop service").Log();
      return 1;
    }

    if (!battle_agent.AddService(std::make_unique<kero::SocketPoolService>())) {
      kero::log::Error("Agent failed to add socket pool service").Log();
      return 1;
    }
  }

  kero::ThreadAgent lobby_thread_agent{};
  if (!lobby_thread_agent.Start(std::move(lobby_agent))) {
    kero::log::Error("Lobby thread agent failed to start").Log();
    return 1;
  }

  kero::Defer defer_lobby_thread_agent{[&lobby_thread_agent] {
    if (!lobby_thread_agent.Stop()) {
      kero::log::Error("Lobby thread agent stop failed").Log();
    }
  }};

  kero::ThreadAgent battle_thread_agent{};
  if (!battle_thread_agent.Start(std::move(battle_agent))) {
    kero::log::Error("Battle thread agent failed to start").Log();
    return 1;
  }

  kero::Defer defer_battle_thread_agent{[&battle_thread_agent] {
    if (!battle_thread_agent.Stop()) {
      kero::log::Error("Battle thread agent stop failed").Log();
    }
  }};

  if (auto res = main_agent.Run(); res.IsErr()) {
    if (res.Err().code == kero::Agent::kInterrupted) {
      kero::log::Info("Agent interrupted").Log();
      return 0;
    }

    kero::log::Error("Agent failed to run").Data("error", res.TakeErr()).Log();
    return 1;
  }

  return 0;
}
