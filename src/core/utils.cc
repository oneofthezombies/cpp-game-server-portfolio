#include "utils.h"

using namespace core;

auto core::ParseArgcArgv(int argc, char **argv) noexcept -> Args {
  return Args{argv, argv + argc};
}

core::Lexer::Lexer(Args &&args) noexcept : args_(std::move(args)) {}

auto core::Lexer::Current() const noexcept -> std::optional<std::string_view> {
  if (index_ >= args_.size()) {
    return std::nullopt;
  }
  return args_[index_];
}

auto core::Lexer::Next() const noexcept -> std::optional<std::string_view> {
  if (index_ + 1 >= args_.size()) {
    return std::nullopt;
  }
  return args_[index_ + 1];
}

auto core::Lexer::Eat() noexcept -> void {
  if (index_ < args_.size()) {
    index_ += 1;
  }
}
