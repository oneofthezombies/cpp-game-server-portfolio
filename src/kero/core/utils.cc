#include "utils.h"

using namespace kero;

auto
kero::OkVoid() -> Result<Void> {
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

kero::StackDefer::~StackDefer() noexcept {
  for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
    (*it)();
  }
}

auto
kero::StackDefer::Push(Fn &&fn) noexcept -> void {
  stack_.push_back(std::move(fn));
}

auto
kero::StackDefer::Cancel() noexcept -> void {
  stack_.clear();
}
