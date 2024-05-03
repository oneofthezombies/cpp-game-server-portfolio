#include "pin_system.h"

using namespace kero;

auto
kero::PinSystem::DestroyAll() noexcept -> void {
  std::lock_guard lock{mutex_};
  for (auto& [pin_data_raw, pin_data_destroyer] : pin_map_) {
    pin_data_destroyer(pin_data_raw);
  }

  pin_map_.clear();
}
