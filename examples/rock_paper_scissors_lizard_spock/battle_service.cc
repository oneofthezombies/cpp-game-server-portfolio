// #include "kero/engine/runner_context.h"
// #include "kero/engine/service.h"
// #include "kero/log/log_builder.h"
// #include "kero/service/constants.h"

// using namespace kero;

// class BattleService final : public Service {
//  public:
//   explicit BattleService(RunnerContextPtr &&runner_context) noexcept
//       : Service{std::move(runner_context),
//                 {101, "battle"},
//                 {kServiceKindSocketPool}} {}

//   auto
//   OnCreate() noexcept -> Result<Void> override {
//     using ResultT = Result<Void>;

//     if (!SubscribeEvent(EventSocketRegister::kEvent)) {
//       return ResultT::Err(Error::From(
//           Dict{}
//               .Set("message", "Failed to subscribe to socket register event")
//               .Take()));
//     }

//     if (!SubscribeEvent(EventSocketUnregister::kEvent)) {
//       return ResultT::Err(Error::From(
//           Dict{}
//               .Set("message", "Failed to subscribe to socket unregister
//               event") .Take()));
//     }

//     return ResultT::Ok(Void{});
//   }

//   auto
//   OnDestroy() noexcept -> void override {
//     if (!UnsubscribeEvent(EventSocketRegister::kEvent)) {
//       log::Error("Failed to unsubscribe from socket register event").Log();
//     }

//     if (!UnsubscribeEvent(EventSocketUnregister::kEvent)) {
//       log::Error("Failed to unsubscribe from socket unregister event").Log();
//     }
//   }

//   auto
//   OnEvent(const std::string &event,
//           const Dict &data) noexcept -> void override {
//     if (event == EventSocketRegister::kEvent) {
//       OnSocketRegister(data);
//     } else if (event == EventSocketUnregister::kEvent) {
//       OnSocketUnregister(data);
//     } else {
//       log::Error("Unknown event").Data("event", event).Log();
//     }
//   }

//  private:
//   auto
//   OnSocketRegister(const Dict &data) noexcept -> void {
//     // TODO: Implement this method
//   }

//   auto
//   OnSocketUnregister(const Dict &data) noexcept -> void {
//     // TODO: Implement this method
//   }
// };
