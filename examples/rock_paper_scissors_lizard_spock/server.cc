#include "battle_service.cc"
#include "kero/core/utils.h"
#include "kero/engine/actor_system.h"
#include "kero/engine/agent.h"
#include "kero/log/center.h"
#include "kero/log/core.h"
#include "kero/log/log_builder.h"
#include "kero/log/transport.h"
#include "kero/service/config_service.h"
#include "kero/service/io_event_loop_service.h"
#include "kero/service/signal_service.h"
#include "kero/service/socket_pool_service.h"
#include "kero/service/socket_router_service.h"
#include "kero/service/tcp_server_service.h"
#include "lobby_service.cc"

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
    auto config_res = kero::ConfigServiceFactory{argc, argv}.Create();
    if (config_res.IsErr()) {
      kero::log::Error("Config service failed to parse args")
          .Data("error", config_res.TakeErr())
          .Log();
      return 1;
    }

    auto config = config_res.TakeOk();
    (void)static_cast<kero::ConfigService*>(config.get())
        ->GetConfig()
        .Set("target_actor", "lobby");
    if (auto res = main_agent.AddService(std::move(config)); !res) {
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

  if (!main_agent.AddService(std::make_unique<kero::SocketRouterService>())) {
    kero::log::Error("Agent failed to add socket router service").Log();
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

    if (!lobby_agent.AddService(lobby_res.TakeOk())) {
      kero::log::Error("Agent failed to add lobby actor service").Log();
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

    if (!lobby_agent.AddService(std::make_unique<LobbyService>())) {
      kero::log::Error("Agent failed to add lobby service").Log();
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

    if (!battle_agent.AddService(battle_res.TakeOk())) {
      kero::log::Error("Agent failed to add battle actor service").Log();
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

    if (!battle_agent.AddService(std::make_unique<BattleService>())) {
      kero::log::Error("Agent failed to add battle service").Log();
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
