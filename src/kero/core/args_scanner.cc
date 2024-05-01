#include "args_scanner.h"

using namespace kero;

kero::ArgsScanner::ArgsScanner(const Args &args) noexcept : args_(args) {}

kero::ArgsScanner::ArgsScanner(Args &&args) noexcept : args_(std::move(args)) {}

auto
kero::ArgsScanner::Current() const noexcept -> OptionRef<const std::string &> {
  if (index_ >= args_.size()) {
    return None;
  }

  return args_[index_];
}

auto
kero::ArgsScanner::Next() const noexcept -> OptionRef<const std::string &> {
  if (index_ + 1 >= args_.size()) {
    return None;
  }
  return args_[index_ + 1];
}

auto
kero::ArgsScanner::Eat() noexcept -> void {
  if (index_ < args_.size()) {
    index_ += 1;
  }
}
