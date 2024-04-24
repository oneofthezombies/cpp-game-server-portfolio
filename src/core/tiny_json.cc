#include "tiny_json.h"

#include <cctype>
#include <iostream>

using namespace core;

TinyJson::TinyJson(
    std::unordered_map<std::string_view, std::string_view> &&data) noexcept
    : data_{std::move(data)} {}

auto TinyJson::Get(const std::string_view key) const noexcept
    -> std::optional<std::string_view> {
  if (auto found = data_.find(key); found != data_.end()) {
    return found->second;
  }

  return std::nullopt;
}

auto TinyJson::Parse(const std::string_view tiny_json) noexcept
    -> std::optional<TinyJson> {
  TinyJsonParser parser{tiny_json};
  return parser.Parse();
}

TinyJsonParser::TinyJsonParser(const std::string_view tiny_json) noexcept
    : tiny_json_{tiny_json} {}

auto TinyJsonParser::Parse() noexcept -> std::optional<TinyJson> {
  if (tiny_json_.empty()) {
    return std::nullopt;
  }

  // parse leading brace
  {
    Trim();
    auto current = Current();
    if (!current) {
      return std::nullopt;
    }

    if (*current != '{') {
      return std::nullopt;
    }

    if (!Consume('{')) {
      return std::nullopt;
    }
  }

  while (true) {
    // parse trailing brace
    {
      Trim();
      auto current = Current();
      if (!current) {
        return std::nullopt;
      }

      if (*current == '}') {
        if (!Consume('}')) {
          return std::nullopt;
        }

        return TinyJson{std::move(data_)};
      }
    }

    // parse key value pair
    {
      auto key_value = ParseKeyValue();
      if (!key_value) {
        return std::nullopt;
      }

      data_.emplace(key_value->first, key_value->second);
    }

    // parse trailing comma
    {
      Trim();
      auto current = Current();
      if (!current) {
        return std::nullopt;
      }

      // allow trailing comma
      if (*current == ',') {
        if (!Consume(',')) {
          return std::nullopt;
        }
      }
    }
  }
}

auto TinyJsonParser::ParseKeyValue() noexcept
    -> std::optional<std::pair<std::string_view, std::string_view>> {
  auto key = ParseKey();
  if (!key) {
    return std::nullopt;
  }

  Trim();
  auto current = Current();
  if (!current) {
    return std::nullopt;
  }

  if (*current != ':') {
    return std::nullopt;
  }

  if (!Consume(':')) {
    return std::nullopt;
  }

  auto value = ParseValue();
  if (!value) {
    return std::nullopt;
  }

  return std::make_pair(*key, *value);
}

auto TinyJsonParser::ParseKey() noexcept -> std::optional<std::string_view> {
  return ParseString();
}

auto TinyJsonParser::ParseValue() noexcept -> std::optional<std::string_view> {
  return ParseString();
}

auto TinyJsonParser::ParseString() noexcept -> std::optional<std::string_view> {
  Trim();
  auto current = Current();
  if (!current) {
    return std::nullopt;
  }

  if (*current != '"') {
    return std::nullopt;
  }

  if (!Consume('"')) {
    return std::nullopt;
  }

  auto start = cursor_;
  while (true) {
    if (!Advance()) {
      return std::nullopt;
    }

    current = Current();
    if (!current) {
      return std::nullopt;
    }

    if (*current == '"') {
      auto str = tiny_json_.substr(start, cursor_ - start);
      if (!Consume('"')) {
        return std::nullopt;
      }

      return str;
    }
  }

  return std::nullopt;
}

auto TinyJsonParser::Current(const std::source_location location) const noexcept
    -> std::optional<char> {
  if (cursor_ >= tiny_json_.size()) {
    Log("Unexpected end of input", location);
    return std::nullopt;
  }

  return tiny_json_[cursor_];
}

auto TinyJsonParser::Consume(const char c,
                             const std::source_location location) noexcept
    -> bool {
  if (cursor_ >= tiny_json_.size()) {
    Log("Unexpected end of input", location);
    return false;
  }

  if (tiny_json_[cursor_] != c) {
    Log("Unexpected character", location);
    return false;
  }

  ++cursor_;
  return true;
}

auto TinyJsonParser::Advance(const std::source_location location) noexcept
    -> bool {
  if (cursor_ >= tiny_json_.size()) {
    Log("Unexpected end of input", location);
    return false;
  }

  ++cursor_;
  return true;
}

auto TinyJsonParser::Trim() noexcept -> void {
  while (cursor_ < tiny_json_.size() &&
         isspace(static_cast<int>(tiny_json_[cursor_]))) {
    ++cursor_;
  }
}

auto TinyJsonParser::Log(const std::string_view message,
                         const std::source_location location) const noexcept
    -> void {
  std::cout << "Log: " << message << " at " << location.file_name() << ":"
            << location.line() << ":" << location.column() << std::endl;
}

auto TinyJsonBuilder::Build() noexcept -> std::string {
  std::ostringstream oss;
  oss << '{';

  for (const auto &[key, value] : data_) {
    oss << '"' << key << '"' << ':' << '"' << value << '"' << ',';
  }

  oss.seekp(-1, std::ios_base::end);

  oss << '}';
  return oss.str();
}
