#include "runner_context.h"

#include "kero/core/utils.h"

using namespace kero;

auto
kero::RunnerContext::GetService(const ServiceKind& service_kind) const noexcept
    -> OptionRef<Service&> {
  return service_map_.GetService(service_kind);
}

auto
kero::RunnerContext::GetService(const ServiceKind::Id service_kind_id)
    const noexcept -> OptionRef<Service&> {
  return service_map_.GetService(service_kind_id);
}

auto
kero::RunnerContext::GetService(const ServiceKind::Name& service_kind_name)
    const noexcept -> OptionRef<Service&> {
  return service_map_.GetService(service_kind_name);
}

auto
kero::RunnerContext::SubscribeEvent(const std::string& event,
                                    const ServiceKind& kind) noexcept
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
  return OkVoid();
}

auto
kero::RunnerContext::UnsubscribeEvent(const std::string& event,
                                      const ServiceKind& kind) noexcept
    -> Result<Void> {
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
  return OkVoid();
}

auto
kero::RunnerContext::InvokeEvent(const std::string& event,
                                 const Json& data) noexcept -> Result<Void> {
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
    auto service = service_map_.GetService(kind.id);
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
