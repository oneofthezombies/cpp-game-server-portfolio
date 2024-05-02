#include "runner.h"

#include "kero/engine/runner_builder.h"
#include "kero/engine/runner_context.h"
#include "kero/engine/service.h"
#include "kero/engine/service_traverser.h"
#include "kero/engine/signal_service.h"
#include "kero/log/log_builder.h"

using namespace kero;

kero::Runner::Runner(const Pinned<RunnerContext> runner_context) noexcept
    : runner_context_{runner_context} {}

auto
kero::Runner::Run() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = CreateServices(); res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  auto signal_service = service_map_.GetService(kServiceKindSignal.id);
  auto is_interrupted = false;
  while (!is_interrupted) {
    if (signal_service) {
      auto signal =
          signal_service.Unwrap().As<SignalService>(kServiceKindSignal.id);
      if (signal) {
        is_interrupted = signal.Unwrap().IsInterrupted();
      }
    }

    UpdateServices();
  }

  DestroyServices();

  if (is_interrupted) {
    return ResultT::Err(Error::From(kInterrupted));
  }

  return OkVoid();
}

auto
kero::Runner::GetService(const ServiceKind::Id service_kind_id) const noexcept
    -> OptionRef<Service&> {
  return service_map_.GetService(service_kind_id);
}

auto
kero::Runner::GetService(const ServiceKind::Name service_kind_name)
    const noexcept -> OptionRef<Service&> {
  return service_map_.GetService(service_kind_name);
}

auto
kero::Runner::HasService(const ServiceKind::Id service_kind_id) const noexcept
    -> bool {
  return service_map_.HasService(service_kind_id);
}

auto
kero::Runner::HasService(
    const ServiceKind::Name service_kind_name) const noexcept -> bool {
  return service_map_.HasService(service_kind_name);
}

auto
kero::Runner::HasServiceIs(const ServiceKind::Id service_kind_id) const noexcept
    -> bool {
  auto service = GetService(service_kind_id);
  if (service.IsNone()) {
    return false;
  }

  return service.Unwrap().Is(service_kind_id);
}

auto
kero::Runner::HasServiceIs(
    const ServiceKind::Name service_kind_name) const noexcept -> bool {
  auto service = GetService(service_kind_name);
  if (service.IsNone()) {
    return false;
  }

  return service.Unwrap().Is(service_kind_name);
}

auto
kero::Runner::SubscribeEvent(const std::string& event, const ServiceKind& kind)
    -> Result<Void> {
  using ResultT = Result<Void>;

  auto it = event_handler_map_.find(event);
  if (it == event_handler_map_.end()) {
    it = event_handler_map_.try_emplace(event).first;
  }

  auto& service_kinds = it->second;
  if (service_kinds.contains(kind)) {
    return ResultT::Err(Json{}
                            .Set("message", "already subscribed")
                            .Set("kind_id", static_cast<double>(kind.id))
                            .Set("kind_name", kind.name)
                            .Take());
  }

  service_kinds.insert(kind);
  return Result<Void>::Ok(Void{});
}

auto
kero::Runner::UnsubscribeEvent(const std::string& event,
                               const ServiceKind& kind) -> Result<Void> {
  using ResultT = Result<Void>;

  auto it = event_handler_map_.find(event);
  if (it == event_handler_map_.end()) {
    return ResultT::Err(
        Json{}.Set("message", "event not found").Set("event", event).Take());
  }

  auto& service_kinds = it->second;
  if (!service_kinds.contains(kind)) {
    return ResultT::Err(Json{}
                            .Set("message", "not subscribed")
                            .Set("kind_id", static_cast<double>(kind.id))
                            .Set("kind_name", kind.name)
                            .Take());
  }

  service_kinds.erase(kind);
  return Result<Void>::Ok(Void{});
}

auto
kero::Runner::InvokeEvent(const std::string& event, const Json& data) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  auto it = event_handler_map_.find(event);
  if (it == event_handler_map_.end() || it->second.empty()) {
    return ResultT::Err(Json{}
                            .Set("message", "no services subscribed to event")
                            .Set("event", event)
                            .Take());
  }

  Json error{};
  i64 i{};
  bool message_set{false};
  for (const auto& kind : it->second) {
    auto service = GetService(kind.id);
    if (service.IsNone()) {
      if (!message_set) {
        message_set = true;
        (void)error.Set("message", "service not found");
      }

      (void)error
          .Set("kind_id_" + std::to_string(i), static_cast<double>(kind.id))
          .Set("kind_name_" + std::to_string(i), kind.name);

      ++i;
      continue;
    }

    service.Unwrap().OnEvent(event, data);
  }

  if (!error.AsRaw().empty()) {
    return ResultT::Err(std::move(error));
  }

  return OkVoid();
}

auto
kero::Runner::CreateServices() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  ServiceTraverser traverser{service_map_.GetServiceMapRaw()};
  auto res = traverser.Traverse([](Service& service) {
    using ResultT = Result<Void>;

    if (auto res = service.OnCreate(); res.IsErr()) {
      return ResultT::Err(res.TakeErr());
    }

    return OkVoid();
  });

  if (res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  return OkVoid();
}

auto
kero::Runner::DestroyServices() noexcept -> void {
  auto& service_map_raw = service_map_.GetServiceMapRaw();
  for (auto& [_, service] : service_map_raw) {
    service->OnDestroy();
  }
  service_map_raw.clear();
  service_map_.GetServiceKindIdMapRaw().clear();
}

auto
kero::Runner::UpdateServices() noexcept -> void {
  for (auto& [_, service] : service_map_.GetServiceMapRaw()) {
    service->OnUpdate();
  }
}

kero::ThreadRunner::ThreadRunner(Pinned<Runner> runner) noexcept
    : runner_{runner} {}

auto
kero::ThreadRunner::Start() -> Result<Void> {
  if (thread_.joinable()) {
    return Result<Void>::Err(
        Json{}.Set("message", "thread already started").Take());
  }

  thread_ = std::thread{ThreadMain, runner_};
  return OkVoid();
}

auto
kero::ThreadRunner::Stop() -> Result<Void> {
  if (!thread_.joinable()) {
    return Result<Void>::Err(
        Json{}.Set("message", "thread not started").Take());
  }

  thread_.join();
  return OkVoid();
}

auto
kero::ThreadRunner::ThreadMain(Pinned<Runner> runner) noexcept -> void {
  if (auto res = runner.Unwrap().Run()) {
    log::Info("Runner finished").Log();
  } else {
    log::Error("Runner failed").Data("error", res.TakeErr()).Log();
  }
}

kero::RunnerBuilder::RunnerBuilder(EngineContext* engine_context,
                                   std::string&& name) noexcept
    : engine_context_{engine_context}, name_{std::move(name)} {}

auto
kero::RunnerBuilder::BuildRunner() const noexcept -> Result<Pinned<Runner>> {
  using ResultT = Result<Pinned<Runner>>;

  auto runner_res = engine_context_->pin_object_system.CreatePinning<Runner>(
      [name = name_]() {
        auto name_ = name;
        return new Runner{std::move(name_)};
      });
  if (runner_res.IsErr()) {
    return ResultT::Err(runner_res.TakeErr());
  }

  auto runner = runner_res.TakeOk();
  auto& service_map = runner.Unwrap().service_map_;
  for (const auto& service_factory : service_factories_) {
    auto service_res = service_factory(std::make_unique<RunnerContext>(runner));
    if (service_res.IsErr()) {
      return ResultT::Err(service_res.TakeErr());
    }

    if (auto res = service_map.AddService(service_res.TakeOk())) {
      return ResultT::Err(res.TakeErr());
    }
  }

  return ResultT::Ok(std::move(runner));
}

auto
kero::RunnerBuilder::BuildThreadRunner() const noexcept
    -> Result<Pinned<ThreadRunner>> {
  using ResultT = Result<Pinned<ThreadRunner>>;

  auto runner_res = BuildRunner();
  if (runner_res.IsErr()) {
    return ResultT::Err(runner_res.TakeErr());
  }

  auto runner = runner_res.TakeOk();
  auto thread_runner_res =
      engine_context_->pin_object_system.CreatePinning<ThreadRunner>(
          [runner = runner]() {
            auto runner_ = runner;
            return new ThreadRunner{runner_};
          });
  if (thread_runner_res.IsErr()) {
    return ResultT::Err(thread_runner_res.TakeErr());
  }

  return ResultT::Ok(thread_runner_res.TakeOk());
}
