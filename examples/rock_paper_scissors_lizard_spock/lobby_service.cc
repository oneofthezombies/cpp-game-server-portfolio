// #include "kero/core/json.h"
// #include "kero/engine/runner_context.h"
// #include "kero/log/log_builder.h"
// #include "kero/service/constants.h"
// #include "kero/service/io_event_loop_service.h"

// using namespace kero;

// class LobbyService final : public Service {
//  public:
//   explicit LobbyService(RunnerContextPtr &&runner_context) noexcept
//       : Service{std::move(runner_context),
//                 {100, "lobby"},
//                 {kServiceKindSocketPool, kServiceKindIoEventLoop}} {}

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
//   OnEvent(const std::string &event, const Dict &data) noexcept
//       -> void override {
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
//     const auto fd = data.TryGetAsDouble(EventSocketRegister::kFd);
//     if (!fd) {
//       log::Error("Failed to get fd from socket register event")
//           .Data("data", data)
//           .Log();
//       return;
//     }

//     const auto &io_event_loop =
//         GetRunnerContext()
//             .GetService(kServiceKindIoEventLoop.id)
//             .Unwrap()
//             .As<IoEventLoopService>(kServiceKindIoEventLoop.id)
//             .Unwrap();

//     auto stringified =
//         TinyJson::Stringify(Dict{}
//                                 .Set("kind", "connect")
//                                 .Set("socket_id",
//                                 std::to_string(fd.Unwrap())) .Take());
//     if (stringified.IsErr()) {
//       log::Error("Failed to stringify connect message")
//           .Data("error", stringified.TakeErr())
//           .Log();
//       return;
//     }

//     if (auto res = io_event_loop.WriteToFd(static_cast<int>(fd.Unwrap()),
//                                            stringified.TakeOk());
//         res.IsErr()) {
//       log::Error("Failed to send connect message to socket")
//           .Data("fd", fd.Unwrap())
//           .Data("error", res.TakeErr())
//           .Log();
//     }
//   }

//   auto
//   OnSocketUnregister(const Dict &data) noexcept -> void {
//     const auto fd = data.TryGetAsDouble(EventSocketUnregister::kFd);
//     if (!fd) {
//       log::Error("Failed to get fd from socket unregister event")
//           .Data("data", data)
//           .Log();
//       return;
//     }
//   }
// };
