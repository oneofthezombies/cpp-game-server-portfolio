#include "utils.h"

using namespace core;

auto core::ParseArgcArgv(int argc, char **argv) noexcept -> Args {
  return Args{argv, argv + argc};
}

core::Tokenizer::Tokenizer(Args &&args) noexcept : args_(std::move(args)) {}

auto core::Tokenizer::Current() const noexcept
    -> std::optional<std::string_view> {
  if (index_ >= args_.size()) {
    return std::nullopt;
  }
  return args_[index_];
}

auto core::Tokenizer::Next() const noexcept -> std::optional<std::string_view> {
  if (index_ + 1 >= args_.size()) {
    return std::nullopt;
  }
  return args_[index_ + 1];
}

auto core::Tokenizer::Eat() noexcept -> void {
  if (index_ < args_.size()) {
    index_ += 1;
  }
}

core::Defer::Defer(std::function<void()> &&fn) noexcept : fn_{std::move(fn)} {}

core::Defer::~Defer() noexcept { fn_(); }