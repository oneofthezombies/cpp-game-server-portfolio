#include "components_linux.h"

#include <signal.h>

#include <atomic>

#include "kero/engine/engine.h"
#include "kero/engine/events.h"

using namespace kero;

namespace {

static std::atomic<bool> sigint{false};

auto
OnSignal(int signal) -> void {
  if (signal == SIGINT) {
    sigint = true;
  }
}

}  // namespace

kero::SignalComponent::SignalComponent() noexcept : Component{"signal"} {}

auto
kero::SignalComponent::OnCreate(Engine& engine) noexcept -> void {
  sigint = false;

  if (signal(SIGINT, OnSignal) == SIG_ERR) {
    // TODO: log error using errno
  }
}

auto
kero::SignalComponent::OnDestroy(Engine& engine) noexcept -> void {
  if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
    // TODO: log error using errno
  }
}

auto
kero::SignalComponent::OnUpdate(Engine& engine) noexcept -> void {
  if (sigint) {
    engine.Dispatch(kEventMailToSend,
                    Dict{}.Set("__to", std::string{"all"}).Take());
    sigint = false;
  }
}
