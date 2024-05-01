#include "utils.h"

using namespace kero;

auto
OkVoid() -> Result<Void> {
  return Result<Void>::Ok(Void{});
}

kero::Defer::Defer(std::function<void()> &&fn) noexcept : fn_{std::move(fn)} {}

kero::Defer::~Defer() noexcept {
  if (fn_) {
    fn_();
  }
}

auto
kero::Defer::Cancel() noexcept -> void {
  fn_ = nullptr;
}
