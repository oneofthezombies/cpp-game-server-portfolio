#include "flat_json_scanner.h"

using namespace kero;

auto
FlatJsonScanner::Push(const std::string_view str) noexcept -> void {
  buffer_ += str;
}

auto
FlatJsonScanner::Pop() noexcept -> Option<std::string> {
  using ResultT = Option<std::string>;

  u64 start = 0;
  u64 end = 0;
  i64 curly_brace_count = 0;
  auto view = std::string_view{buffer_};
  while (end < view.size()) {
    if (view[end] == '{') {
      ++curly_brace_count;
      start = end;
    } else if (view[end] == '}') {
      --curly_brace_count;
    }

    if (curly_brace_count == 0) {
      break;
    }

    ++end;

    // reset
    if (curly_brace_count < 0) {
      start = end;
      curly_brace_count = 0;
    }
  }

  if (curly_brace_count != 0) {
    return None;
  }

  auto result = buffer_.substr(start, end + 1);
  buffer_ = view.substr(end + 1);
  return ResultT::Some(std::move(result));
}
