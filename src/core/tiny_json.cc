#include "tiny_json.h"

#include <cctype>
#include <iostream>

using namespace core;

core::TinyJson::TinyJson(TinyJson::Raw &&raw) noexcept : raw_{std::move(raw)} {}

auto core::TinyJson::Get(const std::string_view key) const noexcept
    -> std::optional<std::string_view> {
  auto found = raw_.find(std::string{key});
  if (found == raw_.end()) {
    return std::nullopt;
  }

  return found->second;
}

auto core::TinyJson::AsRaw() const noexcept -> const Raw & { return raw_; }

auto core::TinyJson::Clone() const noexcept -> TinyJson {
  auto raw = raw_;
  return TinyJson{std::move(raw)};
}

auto core::TinyJson::ToString() const noexcept -> std::string {
  std::ostringstream oss;
  Log(oss);
  return oss.str();
}

auto core::TinyJson::Log(std::ostream &os,
                         std::source_location location) const noexcept -> void {
  os << '{';
  for (const auto &[key, value] : raw_) {
    os << '"' << key << "\":\"" << value << "\",";
  }
  os << "\"source_location\":\"" << location.file_name() << ':'
     << location.line() << ':' << location.column() << '"';
  os << '}';
}

auto core::TinyJson::LogLn(std::ostream &os,
                           std::source_location location) const noexcept
    -> void {
  Log(os, location);
  os << '\n';
}

auto core::TinyJson::Parse(const std::string_view tiny_json) noexcept
    -> std::optional<TinyJson> {
  TinyJsonParser parser{tiny_json};
  return parser.Parse();
}

auto core::operator<<(std::ostream &os, const TinyJson &tiny_json) noexcept
    -> std::ostream & {
  os << "TinyJson{";
  auto it = tiny_json.AsRaw().begin();
  while (it != tiny_json.AsRaw().end()) {
    os << it->first;
    os << "=";
    os << it->second;
    if (std::next(it) != tiny_json.AsRaw().end()) {
      os << ", ";
    }
    ++it;
  }
  os << "}";
  return os;
}

core::TinyJsonParser::TinyJsonParser(const std::string_view tiny_json,
                                     TinyJsonParserOptions &&options) noexcept
    : tiny_json_str_{tiny_json}, options_{std::move(options)} {}

auto core::TinyJsonParser::Parse() noexcept -> std::optional<TinyJson> {
  if (tiny_json_str_.empty()) {
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

  bool last_comma = false;
  while (true) {
    // parse trailing brace
    {
      Trim();
      auto current = Current();
      if (!current) {
        return std::nullopt;
      }

      if (*current == '}') {
        if (!options_.allow_trailing_comma && last_comma) {
          Log("Trailing comma is not allowed", std::source_location::current());
          return std::nullopt;
        }

        if (!Consume('}')) {
          return std::nullopt;
        }

        return TinyJson{std::move(raw_)};
      }
    }

    // parse key value pair
    {
      auto key_value = ParseKeyValue();
      if (!key_value) {
        return std::nullopt;
      }

      raw_.emplace(key_value->first, key_value->second);
    }

    // parse trailing comma
    {
      Trim();
      auto current = Current();
      if (!current) {
        return std::nullopt;
      }

      if (*current == ',') {
        if (!Consume(',')) {
          return std::nullopt;
        }

        last_comma = true;
      } else {
        last_comma = false;
      }
    }
  }
}

auto core::TinyJsonParser::ParseKeyValue() noexcept
    -> std::optional<std::pair<std::string, std::string>> {
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

auto core::TinyJsonParser::ParseKey() noexcept -> std::optional<std::string> {
  return ParseString();
}

auto core::TinyJsonParser::ParseValue() noexcept -> std::optional<std::string> {
  return ParseString();
}

auto core::TinyJsonParser::ParseString() noexcept
    -> std::optional<std::string> {
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

  std::string result;
  auto start = cursor_;

  while (true) {
    current = Current();
    if (!current) {
      return std::nullopt;
    }

    if (*current == '"') {
      if (!Consume('"')) {
        return std::nullopt;
      }
      break;
    } else if (*current == '\\') {
      if (!Advance()) {
        return std::nullopt;
      }
      current = Current();
      if (!current) {
        return std::nullopt;
      }
      switch (*current) {
      case '"':
        result += '"';
        break;
      case '\\':
        result += '\\';
        break;
      case '/':
        result += '/';
        break;
      case 'b':
        result += '\b';
        break;
      case 'f':
        result += '\f';
        break;
      case 'n':
        result += '\n';
        break;
      case 'r':
        result += '\r';
        break;
      case 't':
        result += '\t';
        break;
      default:
        Log("Invalid escape sequence", std::source_location::current());
        return std::nullopt;
      }
    } else {
      result += *current;
    }

    if (!Advance()) {
      return std::nullopt;
    }
  }

  return result;
}

auto core::TinyJsonParser::Current(
    const std::source_location location) const noexcept -> std::optional<char> {
  if (cursor_ >= tiny_json_str_.size()) {
    Log("Unexpected end of input", location);
    return std::nullopt;
  }

  return tiny_json_str_[cursor_];
}

auto core::TinyJsonParser::Consume(const char c,
                                   const std::source_location location) noexcept
    -> bool {
  if (cursor_ >= tiny_json_str_.size()) {
    Log("Unexpected end of input", location);
    return false;
  }

  if (tiny_json_str_[cursor_] != c) {
    Log("Unexpected character", location);
    return false;
  }

  ++cursor_;
  return true;
}

auto core::TinyJsonParser::Advance(const std::source_location location) noexcept
    -> bool {
  if (cursor_ >= tiny_json_str_.size()) {
    Log("Unexpected end of input", location);
    return false;
  }

  ++cursor_;
  return true;
}

auto core::TinyJsonParser::Trim() noexcept -> void {
  while (cursor_ < tiny_json_str_.size() &&
         isspace(static_cast<int>(tiny_json_str_[cursor_]))) {
    ++cursor_;
  }
}

auto core::TinyJsonParser::Log(
    const std::string_view message,
    const std::source_location location) const noexcept -> void {
  std::cout << "TinyJsonParser::Log: " << message << " at "
            << location.file_name() << ":" << location.line() << ":"
            << location.column() << std::endl;
}
