#include "runner.h"

#include "service.h"

using namespace kero;

auto
kero::RunnerContext::SubscribeEvent(const std::string& event,
                                    const Service::Kind& kind) -> Result<Void> {
  return runner_.Unwrap().SubscribeEvent(event, kind);
}

auto
kero::RunnerContext::UnsubscribeEvent(const std::string& event,
                                      const Service::Kind& kind)
    -> Result<Void> {
  return runner_.Unwrap().UnsubscribeEvent(event, kind);
}

auto
kero::RunnerContext::InvokeEvent(const std::string& event,
                                 const Dict& data) noexcept -> Result<Void> {
  return runner_.Unwrap().InvokeEvent(event, data);
}

auto
kero::Runner::Run() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  // if (auto res = ResolveDependencies(); res.IsErr()) {
  //   return ResultT::Err(Error::From(res.TakeErr()));
  // }

  // if (auto res = CreateServices(); res.IsErr()) {
  //   return ResultT::Err(Error::From(res.TakeErr()));
  // }

  // auto signal = GetServiceAs<SignalService>(ServiceKind::kSignal);
  // if (!signal) {
  //   log::Debug("Signal service not found")
  //       .Data("thread", std::this_thread::get_id())
  //       .Log();
  // }

  // auto is_interrupted = false;
  // while (!is_interrupted) {
  //   if (signal) {
  //     is_interrupted = signal.Unwrap().IsInterrupted();
  //   }

  //   UpdateServices();
  // }

  // DestroyServices();

  // if (is_interrupted) {
  //   return ResultT::Err(Error::From(kInterrupted));
  // }

  // log::Debug("Agent stopped").Log();
  // return OkVoid;
}

auto
kero::Runner::GetService(const Service::Kind::Id service_kind_id) const noexcept
    -> OptionRef<Service&> {
  return service_map_.GetService(service_kind_id);
}

auto
kero::Runner::GetService(const Service::Kind::Name service_kind_name)
    const noexcept -> OptionRef<Service&> {
  return service_map_.GetService(service_kind_name);
}

auto
kero::Runner::HasService(const Service::Kind::Id service_kind_id) const noexcept
    -> bool {
  return service_map_.HasService(service_kind_id);
}

auto
kero::Runner::HasService(
    const Service::Kind::Name service_kind_name) const noexcept -> bool {
  return service_map_.HasService(service_kind_name);
}

auto
kero::Runner::SubscribeEvent(const std::string& event,
                             const Service::Kind& kind) -> Result<Void> {
  using ResultT = Result<Void>;

  auto it = event_handler_map_.find(event);
  if (it == event_handler_map_.end()) {
    it = event_handler_map_.try_emplace(event).first;
  }

  auto& service_kinds = it->second;
  if (service_kinds.contains(kind)) {
    return ResultT::Err(Dict{}
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
                               const Service::Kind& kind) -> Result<Void> {
  using ResultT = Result<Void>;

  auto it = event_handler_map_.find(event);
  if (it == event_handler_map_.end()) {
    return ResultT::Err(
        Dict{}.Set("message", "event not found").Set("event", event).Take());
  }

  auto& service_kinds = it->second;
  if (!service_kinds.contains(kind)) {
    return ResultT::Err(Dict{}
                            .Set("message", "not subscribed")
                            .Set("kind_id", static_cast<double>(kind.id))
                            .Set("kind_name", kind.name)
                            .Take());
  }

  service_kinds.erase(kind);
  return Result<Void>::Ok(Void{});
}

auto
kero::Runner::InvokeEvent(const std::string& event, const Dict& data) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  auto it = event_handler_map_.find(event);
  if (it == event_handler_map_.end() || it->second.empty()) {
    return ResultT::Err(Dict{}
                            .Set("message", "no services subscribed to event")
                            .Set("event", event)
                            .Take());
  }

  Dict error{};
  int64_t i{};
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

  return OkVoid;
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

    return OkVoid;
  });

  if (res.IsErr()) {
    return ResultT::Err(res.TakeErr());
  }

  return OkVoid;
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
