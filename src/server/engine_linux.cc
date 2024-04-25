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
#include "mail_center.h"
#include "main_event_loop_linux.h"
#include "utils.h"
#include "utils_linux.h"

std::atomic<const MailBox *> signal_mail_box_ptr{nullptr};

auto OnSignal(int signal) -> void {
  if (signal == SIGINT) {
    // if (signal_mail_box_ptr != nullptr) {
    //   signal_mail_box_ptr->tx.Send(Mail{"signal", "all", {{"shutdown",
    //   ""}}});
    // }
  }

  std::cout << "Signal received: " << signal << std::endl;
}

EngineLinux::EngineLinux(MainEventLoopLinux &&main_event_loop,
                         std::thread &&lobby_thread,
                         std::thread &&battle_thread) noexcept
    : main_event_loop_{std::move(main_event_loop)},
      lobby_thread_{std::move(lobby_thread)},
      battle_thread_{std::move(battle_thread)} {}

auto EngineLinux::Run() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  auto signal_mail_box_res = MailCenter::Global().Create("signal");
  if (signal_mail_box_res.IsErr()) {
    return ResultT{std::move(signal_mail_box_res.Err())};
  }

  const auto &signal_mail_box = signal_mail_box_res.Ok();
  signal_mail_box_ptr.store(&signal_mail_box);

  {
    Defer reset_signal_mail_box_ptr{
        []() { signal_mail_box_ptr.store(nullptr); }};
    if (signal(SIGINT, OnSignal) == SIG_ERR) {
      return ResultT{Error{Symbol::kLinuxSignalSetFailed,
                           SB{}.Add(LinuxError::FromErrno()).Build()}};
    }

    if (auto res = main_event_loop_.Run(); res.IsErr()) {
      return res;
    }

    if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
      return ResultT{Error{Symbol::kLinuxSignalResetFailed,
                           SB{}.Add(LinuxError::FromErrno()).Build()}};
    }
  }

  battle_thread_.join();
  lobby_thread_.join();
  return ResultT{Void{}};
}

auto EngineLinux::Builder::Build(const uint16_t port) const noexcept
    -> Result<EngineLinux> {
  using ResultT = Result<EngineLinux>;

  auto main_event_loop_res = MainEventLoopLinux::Builder{}.Build(port);
  if (main_event_loop_res.IsErr()) {
    return ResultT{std::move(main_event_loop_res.Err())};
  }

  auto &main_event_loop = main_event_loop_res.Ok();
  if (auto res = main_event_loop.Init("main"); res.IsErr()) {
    return ResultT{std::move(res.Err())};
  }

  auto lobby_event_loop = LobbyEventLoopLinux{};
  if (auto res = lobby_event_loop.Init("lobby"); res.IsErr()) {
    return ResultT{std::move(res.Err())};
  }

  auto lobby_thread =
      std::thread{[](LobbyEventLoopLinux &&lobby_event_loop) {
                    if (auto res = lobby_event_loop.Run(); res.IsErr()) {
                      std::cout << res.Err() << std::endl;
                    }
                  },
                  std::move(lobby_event_loop)};

  auto battle_event_loop = BattleEventLoopLinux{};
  if (auto res = battle_event_loop.Init("battle"); res.IsErr()) {
    return ResultT{std::move(res.Err())};
  }

  auto battle_thread =
      std::thread{[](BattleEventLoopLinux &&battle_event_loop) {
                    if (auto res = battle_event_loop.Run(); res.IsErr()) {
                      std::cout << res.Err() << std::endl;
                    }
                  },
                  std::move(battle_event_loop)};

  return ResultT{EngineLinux{std::move(main_event_loop),
                             std::move(lobby_thread),
                             std::move(battle_thread)}};
}
