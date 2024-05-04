#include "runner_context.h"

#include "kero/core/utils.h"

using namespace kero;

kero::RunnerContext::RunnerContext(std::string&& runner_name) noexcept
    : runner_name_{std::move(runner_name)} {}

auto
kero::RunnerContext::SubscribeEvent(
    const std::string& event,
    const ServiceKindId service_kind_id) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  auto it = event_handler_map_.find(event);
  if (it == event_handler_map_.end()) {
    it = event_handler_map_.try_emplace(event).first;
  }

  auto& service_kind_ids = it->second;
  if (service_kind_ids.contains(service_kind_id)) {
    auto service_kind_name_res = service_map_.FindNameById(service_kind_id);
    if (service_kind_name_res.IsErr()) {
      return ResultT::Err(service_kind_name_res.TakeErr());
    }

    const auto service_kind_name = service_kind_name_res.TakeOk();
    return ResultT::Err(FlatJson{}
                            .Set("message", "already subscribed")
                            .Set("service_kind_id", service_kind_id)
                            .Set("service_kind_name", service_kind_name)
                            .Take());
  }

  service_kind_ids.insert(service_kind_id);
  return OkVoid();
}

auto
kero::RunnerContext::UnsubscribeEvent(
    const std::string& event,
    const ServiceKindId service_kind_id) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  auto it = event_handler_map_.find(event);
  if (it == event_handler_map_.end()) {
    return ResultT::Err(FlatJson{}
                            .Set("message", "event not found")
                            .Set("event", event)
                            .Take());
  }

  auto& service_kind_ids = it->second;
  if (!service_kind_ids.contains(service_kind_id)) {
    auto service_kind_name_res = service_map_.FindNameById(service_kind_id);
    if (service_kind_name_res.IsErr()) {
      return ResultT::Err(service_kind_name_res.TakeErr());
    }

    const auto service_kind_name = service_kind_name_res.TakeOk();
    return ResultT::Err(FlatJson{}
                            .Set("message", "not subscribed")
                            .Set("service_kind_id", service_kind_id)
                            .Set("service_kind_name", service_kind_name)
                            .Take());
  }

  service_kind_ids.erase(service_kind_id);
  return OkVoid();
}

auto
kero::RunnerContext::InvokeEvent(
    const std::string& event, const FlatJson& data) noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  auto it = event_handler_map_.find(event);
  if (it == event_handler_map_.end() || it->second.empty()) {
    return ResultT::Err(FlatJson{}
                            .Set("message", "no services subscribed to event")
                            .Set("event", event)
                            .Take());
  }

  FlatJson error{};
  i64 i{};
  bool message_set{false};
  for (const auto service_kind_id : it->second) {
    auto service = service_map_.GetService(service_kind_id);
    if (service.IsNone()) {
      if (!message_set) {
        message_set = true;
        (void)error.Set("message", "service not found");
      }

      auto service_kind_name_res = service_map_.FindNameById(service_kind_id);
      if (service_kind_name_res.IsErr()) {
        return ResultT::Err(service_kind_name_res.TakeErr());
      }

      const auto service_kind_name = service_kind_name_res.TakeOk();
      (void)error.Set("service_kind_id_" + std::to_string(i), service_kind_id)
          .Set("service_kind_name_" + std::to_string(i), service_kind_name);

      ++i;
      continue;
    }

    service.Unwrap()->OnEvent(event, data);
  }

  if (!error.AsRaw().empty()) {
    return ResultT::Err(std::move(error));
  }

  return OkVoid();
}

auto
kero::RunnerContext::GetName() const noexcept -> const std::string& {
  return runner_name_;
}
