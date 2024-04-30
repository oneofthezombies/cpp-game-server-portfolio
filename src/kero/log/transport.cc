#include "transport.h"

#include <iostream>

#include "kero/log/utils.h"

using namespace kero;

kero::Transport::Transport() noexcept : level_{Level::kInfo} {}

kero::Transport::Transport(const Level level) noexcept : level_{level} {}

auto
kero::Transport::SetLevel(const Level level) noexcept -> void {
  level_ = level;
}

auto
kero::Transport::GetLevel() const noexcept -> Level {
  return level_;
}

auto
PrintIndent(std::ostream& os, const size_t indent) -> void {
  for (size_t i = 0; i < indent; ++i) {
    os << " ";
  }
}

auto
kero::ConsolePlainTextTransport::OnLog(const Log& log) noexcept -> void {
  auto& os = std::cout;
  size_t indent{0};
  PrintIndent(os, indent);

  const auto datetime = TimePointToIso8601(std::chrono::system_clock::now());
  os << datetime;
  os << " ";
  os << LevelToString(log.level);
  os << " - ";
  os << log.message;
  os << "\n";
  indent += 2;

  if (!log.data.empty()) {
    PrintIndent(os, indent);
    os << "data:\n";
    const auto data_indent = indent + 2;
    for (auto it = log.data.begin(); it != log.data.end(); ++it) {
      PrintIndent(os, data_indent);
      os << it->first << ": " << it->second;
      if (std::next(it) != log.data.end()) {
        os << "\n";
      }
    }
    os << "\n";
  }

  if (log.location.file_name() != nullptr) {
    PrintIndent(os, indent);
    os << "location: ";
    os << log.location.file_name();
    os << ":";
    os << log.location.line();
    os << ":";
    os << log.location.column();
    os << "\n";
  }

  if (log.location.function_name() != nullptr) {
    PrintIndent(os, indent);
    os << "function: ";
    os << log.location.function_name();
    os << "\n";
  }
}
