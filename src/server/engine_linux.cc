#include "engine_linux.h"

#include <atomic>
#include <iostream>
#include <signal.h>

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "battle_event_loop_linux.h"
#include "common.h"
#include "lobby_event_loop_linux.h"
#include "main_event_loop_linux.h"
#include "spsc_channel.h"
#include "utils.h"
#include "utils_linux.h"

std::atomic<core::Tx<EventLoopLinuxEvent> *> signal_to_main_tx_ptr{};

auto OnSignal(int signal) -> void {
  if (signal == SIGINT) {
    if (signal_to_main_tx_ptr) {
      (*signal_to_main_tx_ptr).Send({{"shutdown", ""}});
    }
  }

  std::cout << "Signal received: " << signal << std::endl;
}

EngineLinux::EngineLinux(MainEventLoopLinux &&main_event_loop,
                         core::Tx<EventLoopLinuxEvent> &&signal_to_main_tx,
                         std::thread &&lobby_thread,
                         std::thread &&battle_thread) noexcept
    : main_event_loop_{std::move(main_event_loop)},
      signal_to_main_tx_{std::move(signal_to_main_tx)},
      lobby_thread_{std::move(lobby_thread)} {}

auto EngineLinux::Run() noexcept -> Result<core::Void> {
  using ResultT = Result<core::Void>;

  signal_to_main_tx_ptr = &signal_to_main_tx_;
  {
    core::Defer reset_tx_ptr{[]() { signal_to_main_tx_ptr = nullptr; }};
    if (signal(SIGINT, OnSignal) == SIG_ERR) {
      return ResultT{Error{Symbol::kLinuxSignalSetFailed,
                           SB{}.Add(core::LinuxError::FromErrno()).Build()}};
    }

    if (auto res = main_event_loop_.Run(); res.IsErr()) {
      return res;
    }

    if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
      return ResultT{Error{Symbol::kLinuxSignalResetFailed,
                           SB{}.Add(core::LinuxError::FromErrno()).Build()}};
    }
  }

  lobby_thread_.join();
  return ResultT{core::Void{}};
}

// auto EngineLinux::OnClientFdEvent(
//     const FileDescriptorLinux::Raw client_fd) noexcept -> Result<core::Void>
//     {
//   char buffer[1024 * 8]{};
//   const auto count = read(client_fd, buffer, sizeof(buffer));
//   if (count == -1) {
//     DeleteConnectedSessionOrCloseFd(client_fd);
//     return Error{Symbol::kEngineLinuxClientSocketReadFailed,
//                  SB{}.Add(core::LinuxError::FromErrno())
//                      .Add("client_fd", client_fd)
//                      .Build()};
//   }

//   if (count == 0) {
//     DeleteConnectedSessionOrCloseFd(client_fd);
//     return Error{Symbol::kEngineLinuxClientSocketClosed,
//                  SB{}.Add("client_fd", client_fd).Build()};
//   }

//   // remove from epoll
//   {
//     struct epoll_event del_client_ev {};
//     del_client_ev.events = EPOLLIN | EPOLLET;
//     del_client_ev.data.fd = client_fd;
//     if (epoll_ctl(epoll_fd_.AsRaw(), EPOLL_CTL_DEL, client_fd,
//                   &del_client_ev) == -1) {
//       return Error{Symbol::kEngineLinuxEpollCtlDeleteClientFailed,
//                    SB{}.Add(core::LinuxError::FromErrno())
//                        .Add("client_fd", client_fd)
//                        .Build()};
//     }
//   }

//   // parse message
//   const std::string_view buffer_view{buffer, static_cast<size_t>(count)};
//   const auto message = core::Message::FromRaw(buffer_view);
//   if (!message) {
//     DeleteConnectedSessionOrCloseFd(client_fd);
//     return Error{Symbol::kEngineLinuxMessageParseFailed,
//                  SB{}.Add("buffer_view", buffer_view)
//                      .Add("client_fd", client_fd)
//                      .Build()};
//   }

//   std::cout << "Received message: " << *message << std::endl;

//   const auto room_id = message->json.Get("room_id");
//   if (!room_id) {
//     DeleteConnectedSessionOrCloseFd(client_fd);
//     return Error{Symbol::kEngineLinuxMessageParseFailed,
//                  SB{}.Add("room_id not found")
//                      .Add("buffer_view", buffer_view)
//                      .Add("client_fd", client_fd)
//                      .Build()};
//   }

//   // write to client_fd message
//   {
//     const auto success =
//         core::Message::BuildRaw(core::MessageKind::kRequestSuccess,
//         message->id,
//                                 core::TinyJsonBuilder{}.Build());
//     if (write(client_fd, success.data(), success.size()) == -1) {
//       DeleteConnectedSessionOrCloseFd(client_fd);
//       return Error{Symbol::kEngineLinuxClientSocketWriteFailed,
//                    SB{}.Add(core::LinuxError::FromErrno())
//                        .Add("client_fd", client_fd)
//                        .Build()};
//     }
//   }

//   return core::Void{};
// }

// auto EngineLinux::DeleteConnectedSessionOrCloseFd(
//     const FileDescriptorLinux::Raw client_fd) noexcept -> void {
//   const auto session_id = FileDescriptorLinux::RawToSessionId(client_fd);
//   auto found_session = connected_sessions_.find(session_id);
//   if (found_session != connected_sessions_.end()) {
//     connected_sessions_.erase(found_session);
//     std::cout << "Session deleted: " << session_id << std::endl;
//     return;
//   }

//   if (close(client_fd) == -1) {
//     const Error error{Symbol::kEngineLinuxClientSocketCloseFailed,
//                       SB{}.Add(core::LinuxError::FromErrno())
//                           .Add("client_fd", client_fd)
//                           .Build()};
//     std::cout << error << std::endl;
//   } else {
//     std::cout << "Client fd closed: " << client_fd << std::endl;
//   }
// }

auto EngineLinuxBuilder::Build(const uint16_t port) const noexcept
    -> Result<EngineLinux> {
  using ResultT = Result<EngineLinux>;

  auto [signal_to_main_tx, signal_to_main_rx] =
      core::Channel<EventLoopLinuxEvent>::Builder{}.Build();
  auto [main_to_lobby_tx, main_to_lobby_rx] =
      core::Channel<EventLoopLinuxEvent>::Builder{}.Build();
  auto [lobby_to_battle_tx, lobby_to_battle_rx] =
      core::Channel<EventLoopLinuxEvent>::Builder{}.Build();
  auto [battle_to_lobby_tx, battle_to_lobby_rx] =
      core::Channel<EventLoopLinuxEvent>::Builder{}.Build();

  auto main_event_loop_res = MainEventLoopLinuxBuilder{}.Build(
      port, std::move(signal_to_main_rx), std::move(main_to_lobby_tx));
  if (main_event_loop_res.IsErr()) {
    return ResultT{std::move(main_event_loop_res.Err())};
  }

  auto lobby_event_loop_res = LobbyEventLoopLinuxBuilder{}.Build(
      std::move(main_to_lobby_rx), std::move(battle_to_lobby_rx),
      std::move(lobby_to_battle_tx));
  if (lobby_event_loop_res.IsErr()) {
    return ResultT{std::move(lobby_event_loop_res.Err())};
  }

  auto lobby_thread =
      std::thread{[](LobbyEventLoopLinux &&lobby_event_loop) {
                    if (auto res = lobby_event_loop.Run(); res.IsErr()) {
                      std::cout << res.Err() << std::endl;
                    }
                  },
                  std::move(lobby_event_loop_res.Ok())};

  auto battle_event_loop_res = BattleEventLoopLinuxBuilder{}.Build(
      std::move(lobby_to_battle_rx), std::move(battle_to_lobby_tx));
  if (battle_event_loop_res.IsErr()) {
    return ResultT{std::move(battle_event_loop_res.Err())};
  }

  auto battle_thread =
      std::thread{[](BattleEventLoopLinux &&battle_event_loop) {
                    if (auto res = battle_event_loop.Run(); res.IsErr()) {
                      std::cout << res.Err() << std::endl;
                    }
                  },
                  std::move(battle_event_loop_res.Ok())};

  return ResultT{EngineLinux{
      std::move(main_event_loop_res.Ok()), std::move(signal_to_main_tx),
      std::move(lobby_thread), std::move(battle_thread)}};
}
