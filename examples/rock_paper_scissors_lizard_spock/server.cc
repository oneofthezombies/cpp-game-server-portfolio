#include <memory>

#include "battle_service.cc"
#include "kero/engine/actor_service.h"
#include "kero/engine/engine.h"
#include "kero/engine/runner_builder.h"
#include "kero/engine/signal_service.h"
#include "kero/log/center.h"
#include "kero/log/core.h"
#include "kero/log/transport.h"
#include "kero/middleware/config_service.h"
#include "kero/middleware/io_event_loop_service.h"
#include "kero/middleware/socket_router_service.h"
#include "kero/middleware/tcp_server_service.h"
#include "match_service.cc"

/**
 * `using namespace kero` is used because it is an example program.
 */
using namespace kero;

auto
RunEngine(int argc, char** argv, Own<Engine>& engine) -> Result<Void>;
auto
BuildMainRunner(int argc,
                char** argv,
                Own<Engine>& engine) -> Result<Pin<Runner>>;
auto
BuildMatchRunner(Own<Engine>& engine) -> Result<Pin<ThreadRunner>>;
auto
BuildBattleRunner(Own<Engine>& engine,
                  const i32 index) -> Result<Pin<ThreadRunner>>;

auto
main(int argc, char** argv) -> int {
  Center{}.UseStreamForLoggingSystemError();
  auto transport = std::make_unique<ConsolePlainTextTransport>();
  transport->SetLevel(Level::kDebug);
  Center{}.AddTransport(std::move(transport));

  auto engine = std::make_unique<Engine>();
  auto run_engine_res = RunEngine(argc, argv, engine);
  if (run_engine_res.IsErr()) {
    log::Error("Failed to run engine")
        .Data("error", run_engine_res.TakeErr())
        .Log();
  }

  Center{}.Shutdown();
  return run_engine_res.IsOk() ? 0 : 1;
}

auto
RunEngine(int argc, char** argv, Own<Engine>& engine) -> Result<Void> {
  using ResultT = Result<Void>;

  StackDefer defer;

  if (auto res = engine->Start(); res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  defer.Push([&engine] {
    if (auto res = engine->Stop(); res.IsErr()) {
      log::Error("Failed to stop engine").Data("error", res.TakeErr()).Log();
    }
  });

  auto match_runner_res = BuildMatchRunner(engine);
  if (match_runner_res.IsErr()) {
    return ResultT::Err(match_runner_res.TakeErr());
  }

  auto match_runner = match_runner_res.Ok();
  if (auto res = match_runner->Start(); res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  defer.Push([match_runner] {
    if (auto res = match_runner->Stop(); res.IsErr()) {
      log::Error("Failed to stop match runner")
          .Data("error", res.TakeErr())
          .Log();
    }
  });

  const auto core_count = std::thread::hardware_concurrency();

  // minimum thread count is 3 (main io, logging, actor system).
  const auto battle_count = core_count - 3 > 0 ? core_count : 1;

  for (i32 i = 0; i < battle_count; ++i) {
    auto battle_runner_res = BuildBattleRunner(engine, i);
    if (battle_runner_res.IsErr()) {
      return ResultT::Err(battle_runner_res.TakeErr());
    }

    auto battle_runner = battle_runner_res.Ok();
    if (auto res = battle_runner->Start(); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    defer.Push([battle_runner] {
      if (auto res = battle_runner->Stop(); res.IsErr()) {
        log::Error("Failed to stop battle runner")
            .Data("error", res.TakeErr())
            .Log();
      }
    });
  }

  auto main_runner_res = BuildMainRunner(argc, argv, engine);
  if (main_runner_res.IsErr()) {
    return ResultT::Err(main_runner_res.TakeErr());
  }

  auto main_runner = main_runner_res.Ok();
  if (auto res = main_runner->Run(); res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  return OkVoid();
}

auto
BuildMainRunner(int argc,
                char** argv,
                Own<Engine>& engine) -> Result<Pin<Runner>> {
  using ResultT = Result<Pin<Runner>>;

  auto res =
      engine->CreateRunnerBuilder("main")
          .AddServiceFactory(std::make_unique<ConfigServiceFactory>(argc, argv))
          .AddServiceFactory(
              std::make_unique<DefaultServiceFactory<SignalService>>())
          .AddServiceFactory(std::make_unique<ActorServiceFactory>(engine))
          .AddServiceFactory(
              std::make_unique<DefaultServiceFactory<IoEventLoopService>>())
          .AddServiceFactory(
              std::make_unique<DefaultServiceFactory<TcpServerService>>())
          .AddServiceFactory([](const Pin<RunnerContext> runner_context) {
            return Result<Own<Service>>{
                std::make_unique<SocketRouterService>(runner_context, "match")};
          })
          .BuildRunner();

  if (res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  return ResultT::Ok(res.TakeOk());
}

auto
BuildMatchRunner(Own<Engine>& engine) -> Result<Pin<ThreadRunner>> {
  using ResultT = Result<Pin<ThreadRunner>>;

  auto res =
      engine->CreateRunnerBuilder("match")
          .AddServiceFactory(std::make_unique<ActorServiceFactory>(engine))
          .AddServiceFactory(
              std::make_unique<DefaultServiceFactory<IoEventLoopService>>())
          .AddServiceFactory(
              std::make_unique<DefaultServiceFactory<MatchService>>())
          .BuildThreadRunner();

  if (res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  return ResultT::Ok(res.TakeOk());
}

auto
BuildBattleRunner(Own<Engine>& engine,
                  const i32 index) -> Result<Pin<ThreadRunner>> {
  using ResultT = Result<Pin<ThreadRunner>>;

  auto res =
      engine->CreateRunnerBuilder("battle:" + std::to_string(index))
          .AddServiceFactory(std::make_unique<ActorServiceFactory>(engine))
          .AddServiceFactory(
              std::make_unique<DefaultServiceFactory<IoEventLoopService>>())
          .AddServiceFactory(
              std::make_unique<DefaultServiceFactory<BattleService>>())
          .BuildThreadRunner();

  if (res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  return ResultT::Ok(res.TakeOk());
}
