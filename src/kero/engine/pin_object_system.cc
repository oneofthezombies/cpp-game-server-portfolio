#include "pin_object_system.h"

using namespace kero;

auto
kero::PinObjectSystem::DestroyAll() noexcept -> void {
  std::lock_guard lock{mutex_};
  for (auto& [raw, deleter] : pin_objects_) {
    deleter(raw);
  }
  pin_objects_.clear();
}
