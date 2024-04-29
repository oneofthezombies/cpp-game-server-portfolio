#include "agent.h"

#include "kero/engine/constants.h"
#include "kero/engine/service.h"
#include "kero/engine/signal_service.h"
#include "kero/log/log_builder.h"

using namespace kero;

auto
kero::Agent::Run() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  auto res = CreateServices();
  if (res.IsErr()) {
    return ResultT::Err(Error::From(res.TakeErr()));
  }

  auto signal = GetServiceAs<SignalService>(ServiceKind::kSignal);
  if (!signal) {
    return ResultT::Err(Error::From(
        Dict{}
            .Set("message", std::string{"Failed to get signal service."})
            .Take()));
  }

  auto is_interrupted = false;
  while (true) {
    if (signal) {
      is_interrupted = signal.Unwrap().IsInterrupted();
    }

    UpdateServices();
  }

  DestroyServices();

  if (is_interrupted) {
    return ResultT::Err(Error::From(kInterrupted));
  }

  return ResultT::Ok(Void{});
}

auto
kero::Agent::Invoke(const std::string& event, const Dict& data) noexcept
    -> void {
  auto it = events_.find(event);
  if (it == events_.end() || it->second.empty()) {
    log::Warn("No services subscribed to event").Data("event", event).Log();
    return;
  }

  for (const auto service_kind : it->second) {
    auto service = GetService(service_kind);
    if (service.IsNone()) {
      log::Warn("Service not found").Data("service", service_kind).Log();
      continue;
    }

    service.Unwrap().OnEvent(*this, event, data);
    log::Debug("Service handled event")
        .Data("service", service_kind)
        .Data("event", event)
        .Data("data", data)
        .Log();
  }
}

auto
kero::Agent::SubscribeEvent(const std::string& event,
                            const Service::Kind service_kind) noexcept -> bool {
  auto it = events_.find(event);
  if (it == events_.end()) {
    it = events_.try_emplace(event).first;
  }

  auto& service_kinds = it->second;
  if (service_kinds.contains(service_kind)) {
    return false;
  }

  service_kinds.insert(service_kind);
  return true;
}

auto
kero::Agent::UnsubscribeEvent(const std::string& event,
                              const Service::Kind service_kind) noexcept
    -> bool {
  auto it = events_.find(event);
  if (it == events_.end()) {
    return false;
  }

  auto& service_kinds = it->second;
  if (!service_kinds.contains(service_kind)) {
    return false;
  }

  service_kinds.erase(service_kind);
  return true;
}

auto
kero::Agent::GetService(const Service::Kind service_kind) const noexcept
    -> OptionRef<Service&> {
  auto it = services_.find(service_kind);
  if (it == services_.end()) {
    return None;
  }

  return OptionRef<Service&>{*it->second};
}

auto
kero::Agent::HasService(const Service::Kind service_kind) const noexcept
    -> bool {
  return services_.contains(service_kind);
}

auto
kero::Agent::CreateServices() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  for (auto& [_, service] : services_) {
    auto res = service->OnCreate(*this);
    if (res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }
  }

  return ResultT::Ok(Void{});
}

auto
kero::Agent::DestroyServices() noexcept -> void {
  for (auto& [_, service] : services_) {
    service->OnDestroy(*this);
  }
}

auto
kero::Agent::UpdateServices() noexcept -> void {
  for (auto& [_, service] : services_) {
    service->OnUpdate(*this);
  }
}

kero::ThreadAgent::~ThreadAgent() noexcept {
  if (IsRunning()) {
    [[maybe_unused]] auto stopped = Stop();
  }
}

auto
kero::ThreadAgent::Start(Agent&& agent) noexcept -> bool {
  if (IsRunning()) {
    return false;
  }

  thread_ = std::thread{ThreadAgent::ThreadMain, std::move(agent)};
  return true;
}

auto
kero::ThreadAgent::Stop() noexcept -> bool {
  if (!IsRunning()) {
    return false;
  }

  thread_.join();
  return true;
}

auto
kero::ThreadAgent::IsRunning() const noexcept -> bool {
  return thread_.joinable();
}

auto
kero::ThreadAgent::ThreadMain(Agent&& agent) -> void {
  if (auto res = agent.Run(); res.IsErr()) {
    log::Error("Agent failed to run").Data("error", res.TakeErr()).Log();
  }
}
