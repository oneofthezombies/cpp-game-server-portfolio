#include "pinning_system.h"

using namespace kero;

auto
kero::PinningSystem::Destroy(Raw raw) noexcept -> void {
  std::lock_guard lock{mutex_};
  auto it = pinned_map_.find(raw);
  if (it == pinned_map_.end()) {
    return;
  }

  it->second(raw);
  pinned_map_.erase(it);
}

auto
kero::PinningSystem::DestroyAll() noexcept -> void {
  std::lock_guard lock{mutex_};
  for (auto& [raw, deleter] : pinned_map_) {
    deleter(raw);
  }

  pinned_map_.clear();
}
