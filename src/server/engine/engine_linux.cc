#include "engine_linux.h"

#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <atomic>
#include <cassert>

#include "common.h"
#include "core/tiny_json.h"
#include "core/utils.h"
#include "core/utils_linux.h"
#include "event_loop_linux.h"
#include "mail_center.h"
#include "main_event_loop_handler_linux.h"

using namespace engine;

std::atomic<const MailBox *> signal_mail_box_ptr{nullptr};

auto
OnSignal(int signal) -> void {
  if (signal == SIGINT) {
    if (signal_mail_box_ptr != nullptr) {
      (*signal_mail_box_ptr)
          .tx.Send(Mail{"signal",
                        "all",
                        core::TinyJson{}.Set("__shutdown", "").Take()});
    }
  }

  core::TinyJson{}
      .Set("message", "signal_received")
      .Set("signal", signal)
      .LogLn();
}

auto
engine::EngineLinux::Builder::Build(Config &&config) const noexcept
    -> Result<EngineLinux> {
  using ResultT = Result<EngineLinux>;

  auto main_event_loop_res = EventLoopLinux::Builder{}.Build(
      "main",
      EventLoopHandlerPtr{new MainEventLoopHandlerLinux{
          std::string{config.primary_event_loop_name}}});
  if (main_event_loop_res.IsErr()) {
    return ResultT{Error::From(main_event_loop_res.TakeErr())};
  }

  return ResultT{
      EngineLinux{std::move(config), std::move(main_event_loop_res.Ok())}};
}

engine::EngineLinux::EngineLinux(Config &&config,
                                 EventLoopPtr &&main_event_loop) noexcept
    : config_{std::move(config)},
      main_event_loop_{std::move(main_event_loop)} {}

auto
engine::EngineLinux::Run() noexcept -> Result<Void> {
  using ResultT = Result<Void>;

  assert(main_event_loop_ != nullptr && "main_event_loop must not be nullptr");

  auto signal_mail_box_res = MailCenter::Global().Create("signal");
  if (signal_mail_box_res.IsErr()) {
    return ResultT{Error::From(signal_mail_box_res.TakeErr())};
  }

  const auto &signal_mail_box = signal_mail_box_res.Ok();
  signal_mail_box_ptr.store(&signal_mail_box);

  {
    core::Defer reset_signal_mail_box_ptr{
        []() { signal_mail_box_ptr.store(nullptr); }};
    if (signal(SIGINT, OnSignal) == SIG_ERR) {
      return ResultT{
          Error::From(kLinuxSignalSetFailed,
                      core::TinyJson{}
                          .Set("linux_error", core::LinuxError::FromErrno())
                          .IntoMap())};
    }

    if (auto res = main_event_loop_->Init(config_); res.IsErr()) {
      return ResultT{Error::From(res.TakeErr())};
    }

    if (auto res = main_event_loop_->Run(); res.IsErr()) {
      return ResultT{Error::From(res.TakeErr())};
    }

    if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
      return ResultT{
          Error::From(kLinuxSignalResetFailed,
                      core::TinyJson{}
                          .Set("linux_error", core::LinuxError::FromErrno())
                          .IntoMap())};
    }
  }

  MailCenter::Global().Shutdown();
  return ResultT{Void{}};
}

auto
engine::EngineLinux::AddEventLoop(std::string &&name,
                                  EventLoopHandlerPtr &&handler) noexcept
    -> Result<Void> {
  using ResultT = Result<Void>;

  if (auto it = event_loop_threads_.find(name);
      it != event_loop_threads_.end()) {
    return ResultT{Error::From(kEngineEventLoopAlreadyExists,
                               core::TinyJson{}.Set("name", name).IntoMap())};
  }

  auto event_loop_res =
      EventLoopLinux::Builder{}.Build(std::string{name}, std::move(handler));
  if (event_loop_res.IsErr()) {
    return ResultT{Error::From(event_loop_res.TakeErr())};
  }

  auto event_loop = std::move(event_loop_res.Ok());
  if (auto res = event_loop->Init(config_); res.IsErr()) {
    return ResultT{Error::From(res.TakeErr())};
  }

  auto event_loop_thread =
      std::thread{EventLoopThreadMain, std::move(event_loop)};
  event_loop_threads_.emplace(std::move(name), std::move(event_loop_thread));
  return ResultT{Void{}};
}

auto
engine::EngineLinux::EventLoopThreadMain(EventLoopPtr &&event_loop) noexcept
    -> void {
  assert(event_loop != nullptr && "event_loop must not be nullptr");

  if (auto res = event_loop->Run(); res.IsErr()) {
    core::TinyJson{}
        .Set("message", "event loop thread run failed")
        .Set("name", event_loop->GetName())
        .Set("error", res.Err())
        .LogLn();
  }
}
