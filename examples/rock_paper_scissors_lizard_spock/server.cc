#include <memory>

#include "battle_service.cc"
#include "kero/core/utils.h"
#include "kero/engine/actor_service.h"
#include "kero/engine/engine.h"
#include "kero/engine/runner_builder.h"
#include "kero/engine/signal_service.h"
#include "kero/log/center.h"
#include "kero/log/core.h"
#include "kero/log/transport.h"
#include "kero/middleware/config_service.h"
#include "lobby_service.cc"

/**
 * `using namespace kero` is used because it is an example program.
 */
using namespace kero;

auto
main(int argc, char** argv) -> int {
  Center{}.UseStreamForLoggingSystemError();
  auto transport = std::make_unique<ConsolePlainTextTransport>();
  transport->SetLevel(Level::kDebug);
  Center{}.AddTransport(std::move(transport));
  Defer defer_log_system{[] { Center{}.Shutdown(); }};

  auto engine = std::make_unique<Engine>();
  auto main_runner =
      engine->CreateRunnerBuilder("main")
          .AddServiceFactory(std::make_unique<ConfigServiceFactory>(argc, argv))
          .AddServiceFactory([](const Pin<RunnerContext> runner_context) {
            return Result<Own<Service>>{
                std::make_unique<SignalService>(runner_context)};
          })
          .AddServiceFactory(std::make_unique<ActorServiceFactory>(engine))
          .BuildRunner();

  //   auto main =
  //       kero::Engine::Global()
  //           .CreateRunnerBuilder("main")
  //           .AddServiceFactory(std::make_unique<ConfigServiceFactory>(argc,
  //           argv))
  //           .AddServiceFactory(std::make_unique<SignalServiceFactory>())
  //           .AddServiceFactory(std::make_unique<ActorServiceFactory>())
  //           .AddServiceFactory(std::make_unique<IoEventLoopServiceFactory>())
  //           .AddServiceFactory(std::make_unique<TcpServerServiceFactory>())
  //           .AddServiceFactory(
  //               std::make_unique<SocketRouterServiceFactory>("lobby"))
  //           .BuildRunner();

  //   auto lobby =
  //       kero::Engine::Global()
  //           .CreateRunnerBuilder("lobby")
  //           .AddServiceFactory(std::make_unique<ActorServiceFactory>())
  //           .AddServiceFactory(std::make_unique<IoEventLoopServiceFactory>())
  //           .AddServiceFactory(std::make_unique<SocketPoolServiceFactory>())
  //           .AddServiceFactory(std::make_unique<LobbyServiceFactory>())
  //           .BuildThreadRunner();

  //   auto lobby =
  //       kero::Engine::Global()
  //           .CreateRunnerBuilder("battle")
  //           .AddServiceFactory(std::make_unique<ActorServiceFactory>())
  //           .AddServiceFactory(std::make_unique<IoEventLoopServiceFactory>())
  //           .AddServiceFactory(std::make_unique<SocketPoolServiceFactory>())
  //           .AddServiceFactory(std::make_unique<BattleServiceFactory>())
  //           .BuildThreadRunner();

  return 0;
}
