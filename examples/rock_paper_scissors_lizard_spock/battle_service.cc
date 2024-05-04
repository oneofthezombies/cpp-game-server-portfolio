#include "common.h"
#include "kero/engine/runner_context.h"
#include "kero/engine/service.h"
#include "kero/log/log_builder.h"
#include "kero/middleware/constants.h"

using namespace kero;

class BattleService final : public Service {
 public:
  explicit BattleService(const Pin<RunnerContext> runner_context) noexcept
      : Service{runner_context, {kServiceKindIdSocketPool}} {}

  auto
  OnCreate() noexcept -> Result<Void> override {
    using ResultT = Result<Void>;

    if (!SubscribeEvent(EventBattleStart::kEvent)) {
      return ResultT::Err(Error::From(
          FlatJson{}
              .Set("message", "Failed to subscribe to battle start event")
              .Take()));
    }

    return ResultT::Ok(Void{});
  }

  auto
  OnDestroy() noexcept -> void override {
    if (!UnsubscribeEvent(EventBattleStart::kEvent)) {
      log::Error("Failed to unsubscribe from battle start event").Log();
    }
  }

  auto
  OnEvent(const std::string &event,
          const FlatJson &data) noexcept -> void override {
    if (event == EventBattleStart::kEvent) {
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
