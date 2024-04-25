#include "utils.h"

auto ParseArgcArgv(int argc, char **argv) noexcept -> Args {
  return Args{argv, argv + argc};
}

Tokenizer::Tokenizer(Args &&args) noexcept : args_(std::move(args)) {}

auto Tokenizer::Current() const noexcept -> std::optional<std::string_view> {
  if (index_ >= args_.size()) {
    return std::nullopt;
  }
  return args_[index_];
}

auto Tokenizer::Next() const noexcept -> std::optional<std::string_view> {
  if (index_ + 1 >= args_.size()) {
    return std::nullopt;
  }
  return args_[index_ + 1];
}

auto Tokenizer::Eat() noexcept -> void {
  if (index_ < args_.size()) {
    index_ += 1;
  }
}

Defer::Defer(std::function<void()> &&fn) noexcept : fn_{std::move(fn)} {}

Defer::~Defer() noexcept {
  if (fn_) {
    fn_();
  }
}

auto Defer::Cancel() noexcept -> void { fn_ = nullptr; }
