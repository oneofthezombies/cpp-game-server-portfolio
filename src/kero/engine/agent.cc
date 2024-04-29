#include "agent.h"

#include <unordered_map>

#include "kero/engine/constants.h"
#include "kero/engine/service.h"
#include "kero/engine/signal_service.h"
#include "kero/log/log_builder.h"

using namespace kero;

auto
kero::Agent::Run() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto res = ResolveDependencies(); res.IsErr()) {
    return ResultT::Err(Error::From(res.TakeErr()));
  }

  if (auto res = CreateServices(); res.IsErr()) {
    return ResultT::Err(Error::From(res.TakeErr()));
  }

  auto signal = GetServiceAs<SignalService>(ServiceKind::kSignal);
  if (!signal) {
    log::Debug("Signal service not found")
        .Data("thread", std::this_thread::get_id())
        .Log();
  }

  auto is_interrupted = false;
  while (!is_interrupted) {
    if (signal) {
      is_interrupted = signal.Unwrap().IsInterrupted();
    }

    UpdateServices();
  }

  DestroyServices();

  if (is_interrupted) {
    return ResultT::Err(Error::From(kInterrupted));
  }

  log::Debug("Agent stopped").Log();
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
kero::Agent::ResolveDependencies() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  // Log dependencies.
  for (auto& [_, service] : services_) {
    log::Debug("Resolving dependencies")
        .Data("service", service->GetKind())
        .Data("dependencies", service->GetDependencies())
        .Log();
  }

  // Check dependencies are exist.
  {
    std::vector<
        std::pair<Service::Kind /* service */, Service::Kind /* dependency */>>
        not_founds;
    for (auto& [_, service] : services_) {
      for (const auto dependency : service->GetDependencies()) {
        if (!HasService(dependency)) {
          not_founds.emplace_back(service->GetKind(), dependency);
        }
      }
    }

    if (!not_founds.empty()) {
      std::string not_founds_str;
      for (auto it = not_founds.begin(); it != not_founds.end(); ++it) {
        not_founds_str +=
            std::to_string(it->first) + " -> " + std::to_string(it->second);
        if (std::next(it) != not_founds.end()) {
          not_founds_str += ", ";
        }
      }

      return ResultT::Err(Error::From(
          Dict{}
              .Set("message", std::string{"Failed to resolve dependencies."})
              .Set("not_founds", std::move(not_founds_str))
              .Take()));
    }
  }

  // Check circular dependencies.
  {
    std::unordered_map<Service::Kind, bool> visited;
    for (auto& [_, service] : services_) {
      visited[service->GetKind()] = false;
    }

    std::vector<Service::Kind> stack;

    std::function<Result<Void>(Service::Kind,
                               std::unordered_map<Service::Kind, bool>&,
                               std::vector<Service::Kind>&)>
        dfs = [&](Service::Kind service_kind,
                  std::unordered_map<Service::Kind, bool>& visited,
                  std::vector<Service::Kind>& stack) -> Result<Void> {
      if (visited[service_kind]) {
        return ResultT::Ok(Void{});
      }

      visited[service_kind] = true;
      stack.push_back(service_kind);

      const auto dependencies =
          GetService(service_kind).Unwrap().GetDependencies();
      for (const auto dependency : dependencies) {
        if (!visited[dependency]) {
          if (auto res = dfs(dependency, visited, stack); res.IsErr()) {
            return ResultT::Err(Error::From(res.TakeErr()));
          }
        } else {
          if (auto found = std::find(stack.begin(), stack.end(), dependency);
              found != stack.end()) {
            std::string circular_dependencies;
            for (auto it = std::next(found); it != stack.end(); ++it) {
              circular_dependencies += std::to_string(*it);
              if (std::next(it) != stack.end()) {
                circular_dependencies += " -> ";
              }
            }

            return ResultT::Err(Error::From(
                Dict{}
                    .Set("message", std::string{"Circular dependencies found."})
                    .Set("circular_dependencies",
                         std::move(circular_dependencies))
                    .Take()));
          }
        }
      }

      stack.pop_back();
      return ResultT::Ok(Void{});
    };

    for (auto& [_, service] : services_) {
      if (!visited[service->GetKind()]) {
        if (auto res = dfs(service->GetKind(), visited, stack); res.IsErr()) {
          return ResultT::Err(Error::From(res.TakeErr()));
        }
      }
    }
  }

  return ResultT::Ok(Void{});
}

auto
kero::Agent::CreateServices() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  std::unordered_map<Service::Kind, bool> created;
  for (auto& [_, service] : services_) {
    created[service->GetKind()] = false;
  }

  std::function<Result<Void>(Service::Kind,
                             std::unordered_map<Service::Kind, bool>&)>
      dfs = [&](Service::Kind service_kind,
                std::unordered_map<Service::Kind, bool>& created)
      -> Result<Void> {
    if (created[service_kind]) {
      return ResultT::Ok(Void{});
    }

    const auto dependencies =
        GetService(service_kind).Unwrap().GetDependencies();
    for (const auto dependency : dependencies) {
      if (!created[dependency]) {
        if (auto res = dfs(dependency, created); res.IsErr()) {
          return ResultT::Err(Error::From(res.TakeErr()));
        }
      }
    }

    log::Debug("Creating service").Data("service", service_kind).Log();
    if (auto res = GetService(service_kind).Unwrap().OnCreate(*this);
        res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }

    created[service_kind] = true;
    return ResultT::Ok(Void{});
  };

  for (auto& [_, service] : services_) {
    if (auto res = dfs(service->GetKind(), created); res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }
  }

  log::Debug("All services created").Log();
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
    log::Error("Thread agent failed to run").Data("error", res.TakeErr()).Log();
  }

  log::Debug("Thread agent stopped").Log();
}
