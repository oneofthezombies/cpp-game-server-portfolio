#include "common.h"
#include "kero/core/utils.h"
#include "kero/engine/runner_context.h"
#include "kero/engine/service.h"
#include "kero/log/log_builder.h"
#include "kero/middleware/common.h"

using namespace kero;

class BattleService final : public Service {
 public:
  explicit BattleService(const Pin<RunnerContext> runner_context) noexcept
      : Service{runner_context, {kServiceKindIdSocketPool}} {}

  auto
  OnCreate() noexcept -> Result<Void> override {
    using ResultT = Result<Void>;

    if (!SubscribeEvent(EventSocketRegister::kEvent)) {
      return ResultT::Err(Error::From(
          FlatJson{}
              .Set("message", "Failed to subscribe to socket register event")
              .Take()));
    }

    if (!SubscribeEvent(EventSocketUnregister::kEvent)) {
      return ResultT::Err(Error::From(
          FlatJson{}
              .Set("message", "Failed to subscribe to socket unregister event")
              .Take()));
    }

    return OkVoid();
  }

  auto
  OnDestroy() noexcept -> void override {
    if (!UnsubscribeEvent(EventSocketRegister::kEvent)) {
      log::Error("Failed to unsubscribe from socket register event").Log();
    }

    if (!UnsubscribeEvent(EventSocketUnregister::kEvent)) {
      log::Error("Failed to unsubscribe from socket unregister event").Log();
    }
  }

  auto
  OnEvent(const std::string &event,
          const FlatJson &data) noexcept -> void override {
    if (event == EventSocketRegister::kEvent) {
    } else if (event == EventSocketUnregister::kEvent) {
    } else {
      log::Error("Unknown event").Data("event", event).Log();
    }
  }

 private:
  auto
  NextBattleId() noexcept -> u64 {
    return battle_id_++;
  }

  u64 battle_id_{};
};
