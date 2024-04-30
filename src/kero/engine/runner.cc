#include "runner.h"

using namespace kero;

auto
kero::RunnerContext::SubscribeEvent(const std::string& event,
                                    const Service::Kind& kind) -> Result<Void> {
  return runner_->SubscribeEvent(event, kind);
}

auto
kero::RunnerContext::UnsubscribeEvent(const std::string& event,
                                      const Service::Kind& kind)
    -> Result<Void> {
  return runner_->UnsubscribeEvent(event, kind);
}

auto
kero::RunnerContext::InvokeEvent(const std::string& event,
                                 const Dict& data) noexcept -> void {
  runner_->InvokeEvent(event, data);
}

auto
kero::Runner::SubscribeEvent(const std::string& event,
                             const Service::Kind& kind) -> Result<Void> {
  using ResultT = Result<Void>;

  auto it = events_.find(event);
  if (it == events_.end()) {
    it = events_.try_emplace(event).first;
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

  auto it = events_.find(event);
  if (it == events_.end()) {
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

  auto it = events_.find(event);
  if (it == events_.end() || it->second.empty()) {
    return ResultT::Err(Dict{}
                            .Set("message", "no services subscribed to event")
                            .Set("event", event)
                            .Take());
  }

  Dict error;
  int64_t i{};
  bool message_set{false};
  for (const auto& kind : it->second) {
    auto it = services_.find(kind.id);
    if (it == services_.end()) {
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

    it->second->OnEvent(event, data);
  }

  if (!error.AsRaw().empty()) {
    return ResultT::Err(std::move(error));
  }

  return ResultT::Ok(Void{});
}
