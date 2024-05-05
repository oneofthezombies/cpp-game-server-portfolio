#include "flat_json_parser.h"

using namespace kero;

auto
kero::FlatJsonStringifier::Stringify(const FlatJson& json) noexcept
    -> Result<std::string> {
  using ResultT = Result<std::string>;

  std::string str;
  str += "{";

  for (auto it = json.AsRaw().begin(); it != json.AsRaw().end(); ++it) {
    const auto& key = it->first;
    const auto& value = it->second;

    str += "\"";
    str += key;
    str += "\":";

    if (std::holds_alternative<bool>(value)) {
      str += std::get<bool>(value) ? "true" : "false";
    } else if (std::holds_alternative<double>(value)) {
      const auto double_str = std::to_string(std::get<double>(value));
      str += TrimDoubleString(double_str);
    } else if (std::holds_alternative<std::string>(value)) {
      const auto data = std::get<std::string>(value);
      str += "\"";
      for (const auto c : data) {
        switch (c) {
          case '"':
            str += "\\\"";
            break;
          case '\\':
            str += "\\\\";
            break;
          case '/':
            str += "\\/";
            break;
          case '\b':
            str += "\\b";
            break;
          case '\f':
            str += "\\f";
            break;
          case '\n':
            str += "\\n";
            break;
          case '\r':
            str += "\\r";
            break;
          case '\t':
            str += "\\t";
            break;
          default:
            str += c;
            break;
        }
      }
      str += "\"";
    } else {
      return ResultT::Err(Error::From(FlatJson{}
                                          .Set("kind", "stringify")
                                          .Set("message", "unsupported value")
                                          .Set("key", key)
                                          .Take()));
    }

    if (std::next(it) != json.AsRaw().end()) {
      str += ",";
    }
  }

  str += "}";
  return str;
}

auto
kero::FlatJsonStringifier::TrimDoubleString(const std::string_view str) noexcept
    -> std::string_view {
  if (str.find('.') == std::string::npos) {
    return str;
  }

  size_t pos = str.size() - 1;
  while (str[pos] == '0') {
    --pos;
  }

  if (str[pos] == '.') {
    return str.substr(0, pos);
  }

  return str.substr(0, pos + 1);
}

auto
kero::FlatJsonParser::Parse(const std::string_view tiny_json_str,
                            ParseOptions&& options) noexcept
    -> Result<FlatJson> {
  tiny_json_str_ = tiny_json_str;
  cursor_ = 0;
  options_ = std::move(options);

  auto res = ParseObject();
  if (res.IsErr()) {
    return Result<FlatJson>::Err(Error::From(res.TakeErr()));
  }

  Trim();
  if (cursor_ < tiny_json_str_.size()) {
    return Result<FlatJson>::Err(
        Error::From(FlatJson{}
                        .Set("kind", "parse")
                        .Set("message", "trailing characters")
                        .Set("cursor", cursor_)
                        .Take()));
  }

  return Result<FlatJson>::Ok(res.TakeOk());
}

auto
kero::FlatJsonParser::ParseObject() noexcept -> Result<FlatJson> {
  using ResultT = Result<FlatJson>;

  Trim();
  auto current = Current();
  if (current.IsErr()) {
    return ResultT::Err(Error::From(current.TakeErr()));
  }

  if (current.Ok() != '{') {
    return ResultT::Err(Error::From(FlatJson{}
                                        .Set("kind", "object")
                                        .Set("message", "invalid object")
                                        .Set("cursor", cursor_)
                                        .Take()));
  }

  if (auto res = Advance(); res.IsErr()) {
    return ResultT::Err(Error::From(res.TakeErr()));
  }

  FlatJson::Data data;
  while (true) {
    Trim();
    current = Current();
    if (current.IsErr()) {
      return ResultT::Err(Error::From(current.TakeErr()));
    }

    if (current.Ok() == '}') {
      if (auto res = Advance(); res.IsErr()) {
        return ResultT::Err(Error::From(res.TakeErr()));
      }
      break;
    }

    if (!data.empty()) {
      if (auto res = Consume(','); res.IsErr()) {
        return ResultT::Err(Error::From(res.TakeErr()));
      }
    }

    auto key = ParseKey();
    if (key.IsErr()) {
      return ResultT::Err(Error::From(key.TakeErr()));
    }

    Trim();
    if (auto res = Consume(':'); res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }

    auto value = ParseValue();
    if (value.IsErr()) {
      return ResultT::Err(Error::From(value.TakeErr()));
    }

    data.emplace(std::move(key.TakeOk()), std::move(value.TakeOk()));
  }

  return ResultT::Ok(FlatJson{std::move(data)});
}

auto
kero::FlatJsonParser::ParseKey() noexcept -> Result<std::string> {
  using ResultT = Result<std::string>;

  auto key = ParseString();
  if (key.IsErr()) {
    return ResultT::Err(Error::From(key.TakeErr()));
  }

  return ResultT::Ok(key.TakeOk());
}

auto
kero::FlatJsonParser::ParseValue() noexcept -> Result<FlatJson::ValueStorage> {
  using ResultT = Result<FlatJson::ValueStorage>;

  Trim();
  auto current = Current();
  if (current.IsErr()) {
    return ResultT::Err(Error::From(current.TakeErr()));
  }

  switch (current.Ok()) {
    case '"': {
      if (auto res = ParseString(); res.IsErr()) {
        return ResultT::Err(Error::From(res.TakeErr()));
      } else {
        return ResultT::Ok(FlatJson::ValueStorage{res.TakeOk()});
      }
    }
    case '-':
    case '0' ... '9':
    case '.':
    case 'e':
    case 'E': {
      if (auto res = ParseNumber(); res.IsErr()) {
        return ResultT::Err(Error::From(res.TakeErr()));
      } else {
        return ResultT::Ok(FlatJson::ValueStorage{res.TakeOk()});
      }
    }
    case 't':
    case 'f': {
      if (auto res = ParseBool(); res.IsErr()) {
        return ResultT::Err(Error::From(res.TakeErr()));
      } else {
        return ResultT::Ok(FlatJson::ValueStorage{res.TakeOk()});
      }
    }
    default: {
      return ResultT::Err(Error::From(FlatJson{}
                                          .Set("kind", "value")
                                          .Set("message", "invalid value")
                                          .Set("cursor", cursor_)
                                          .Take()));
    }
  }

  return ResultT::Err(Error::From(FlatJson{}
                                      .Set("kind", "value")
                                      .Set("message", "invalid value")
                                      .Set("cursor", cursor_)
                                      .Take()));
}

auto
kero::FlatJsonParser::ParseString() noexcept -> Result<std::string> {
  using ResultT = Result<std::string>;

  Trim();
  auto current = Current();
  if (current.IsErr()) {
    return ResultT::Err(Error::From(current.TakeErr()));
  }

  if (current.Ok() != '"') {
    return ResultT::Err(Error::From(FlatJson{}
                                        .Set("kind", "string")
                                        .Set("message", "invalid string")
                                        .Set("cursor", cursor_)
                                        .Take()));
  }

  std::string str;
  if (auto res = Advance(); res.IsErr()) {
    return ResultT::Err(Error::From(res.TakeErr()));
  }

  current = Current();
  if (current.IsErr()) {
    return ResultT::Err(Error::From(current.TakeErr()));
  }

  while (current.Ok() != '"') {
    if (current.Ok() == '\\') {
      if (auto res = Advance(); res.IsErr()) {
        return ResultT::Err(Error::From(res.TakeErr()));
      }

      current = Current();
      if (current.IsErr()) {
        return ResultT::Err(Error::From(current.TakeErr()));
      }

      switch (current.Ok()) {
        case '"':
          str += '"';
          break;
        case '\\':
          str += '\\';
          break;
        case '/':
          str += '/';
          break;
        case 'b':
          str += '\b';
          break;
        case 'f':
          str += '\f';
          break;
        case 'n':
          str += '\n';
          break;
        case 'r':
          str += '\r';
          break;
        case 't':
          str += '\t';
          break;
        case 'u': {
          if (auto res = Advance(4); res.IsErr()) {
            return ResultT::Err(Error::From(res.TakeErr()));
          }
          break;
        }
        default:
          return ResultT::Err(Error::From(FlatJson{}
                                              .Set("kind", "string")
                                              .Set("message", "invalid escape")
                                              .Set("cursor", cursor_)
                                              .Take()));
      }
    } else {
      str += current.Ok();
    }

    if (auto res = Advance(); res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }

    current = Current();
    if (current.IsErr()) {
      return ResultT::Err(Error::From(current.TakeErr()));
    }

    if (current.Ok() == '\0') {
      return ResultT::Err(Error::From(FlatJson{}
                                          .Set("kind", "string")
                                          .Set("message", "unterminated string")
                                          .Set("cursor", cursor_)
                                          .Take()));
    }

    if (current.Ok() == '"') {
      break;
    }
  }

  if (auto res = Advance(); res.IsErr()) {
    return ResultT::Err(Error::From(res.TakeErr()));
  }

  return ResultT::Ok(std::move(str));
}

auto
kero::FlatJsonParser::ParseNumber() noexcept -> Result<double> {
  using ResultT = Result<double>;

  Trim();
  auto current = Current();
  if (current.IsErr()) {
    return ResultT::Err(Error::From(current.TakeErr()));
  }

  std::string number;
  if (current.Ok() == '-') {
    number += '-';
    if (auto res = Advance(); res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }
  }

  current = Current();
  if (current.IsErr()) {
    return ResultT::Err(Error::From(current.TakeErr()));
  }

  if (current.Ok() == '0') {
    number += '0';
    if (auto res = Advance(); res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }
  } else if (isdigit(current.Ok())) {
    while (isdigit(current.Ok())) {
      number += current.Ok();
      if (auto res = Advance(); res.IsErr()) {
        return ResultT::Err(Error::From(res.TakeErr()));
      }
      current = Current();
      if (current.IsErr()) {
        return ResultT::Err(Error::From(current.TakeErr()));
      }
    }
  } else {
    return ResultT::Err(Error::From(FlatJson{}
                                        .Set("kind", "number")
                                        .Set("message", "invalid number")
                                        .Set("cursor", cursor_)
                                        .Take()));
  }

  if (current.Ok() == '.') {
    number += '.';
    if (auto res = Advance(); res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }

    current = Current();
    if (current.IsErr()) {
      return ResultT::Err(Error::From(current.TakeErr()));
    }

    if (!isdigit(current.Ok())) {
      return ResultT::Err(Error::From(FlatJson{}
                                          .Set("kind", "number")
                                          .Set("message", "invalid number")
                                          .Set("cursor", cursor_)
                                          .Take()));
    }

    while (isdigit(current.Ok())) {
      number += current.Ok();
      if (auto res = Advance(); res.IsErr()) {
        return ResultT::Err(Error::From(res.TakeErr()));
      }
      current = Current();
      if (current.IsErr()) {
        return ResultT::Err(Error::From(current.TakeErr()));
      }
    }
  }

  if (current.Ok() == 'e' || current.Ok() == 'E') {
    number += current.Ok();
    if (auto res = Advance(); res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }

    current = Current();
    if (current.IsErr()) {
      return ResultT::Err(Error::From(current.TakeErr()));
    }

    if (current.Ok() == '+' || current.Ok() == '-') {
      number += current.Ok();
      if (auto res = Advance(); res.IsErr()) {
        return ResultT::Err(Error::From(res.TakeErr()));
      }
    }

    current = Current();
    if (current.IsErr()) {
      return ResultT::Err(Error::From(current.TakeErr()));
    }

    if (!isdigit(current.Ok())) {
      return ResultT::Err(Error::From(FlatJson{}
                                          .Set("kind", "number")
                                          .Set("message", "invalid number")
                                          .Set("cursor", cursor_)
                                          .Take()));
    }

    while (isdigit(current.Ok())) {
      number += current.Ok();
      if (auto res = Advance(); res.IsErr()) {
        return ResultT::Err(Error::From(res.TakeErr()));
      }
      current = Current();
      if (current.IsErr()) {
        return ResultT::Err(Error::From(current.TakeErr()));
      }
    }
  }

  return ResultT::Ok(std::stod(number));
}

auto
kero::FlatJsonParser::ParseBool() noexcept -> Result<bool> {
  using ResultT = Result<bool>;

  Trim();
  auto current = Current();
  if (current.IsErr()) {
    return ResultT::Err(Error::From(current.TakeErr()));
  }

  if (current.Ok() == 't') {
    if (auto res = Advance(4); res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }
    return ResultT::Ok(true);
  } else if (current.Ok() == 'f') {
    if (auto res = Advance(5); res.IsErr()) {
      return ResultT::Err(Error::From(res.TakeErr()));
    }
    return ResultT::Ok(false);
  }

  return ResultT::Err(Error::From(FlatJson{}
                                      .Set("kind", "bool")
                                      .Set("message", "invalid bool")
                                      .Set("cursor", cursor_)
                                      .Take()));
}

auto
kero::FlatJsonParser::Current() const noexcept -> Result<char> {
  using ResultT = Result<char>;

  if (cursor_ >= tiny_json_str_.size()) {
    return ResultT::Err(Error::From(FlatJson{}
                                        .Set("kind", "current")
                                        .Set("message", "current out of range")
                                        .Set("cursor", cursor_)
                                        .Take()));
  }

  auto c = tiny_json_str_[cursor_];
  return ResultT::Ok(std::move(c));
}

auto
kero::FlatJsonParser::Consume(const char c) noexcept -> Result<bool> {
  using ResultT = Result<bool>;

  if (cursor_ >= tiny_json_str_.size()) {
    return ResultT::Err(Error::From(FlatJson{}
                                        .Set("kind", "consume")
                                        .Set("message", "consume out of range")
                                        .Set("cursor", cursor_)
                                        .Take()));
  }

  const auto actual = tiny_json_str_[cursor_];
  if (actual != c) {
    return ResultT::Err(Error::From(FlatJson{}
                                        .Set("kind", "consume")
                                        .Set("message", "consume mismatch")
                                        .Set("cursor", cursor_)
                                        .Set("expected", c)
                                        .Set("actual", actual)
                                        .Take()));
  }

  ++cursor_;
  return ResultT::Ok(true);
}

auto
kero::FlatJsonParser::Advance(const size_t n) noexcept -> Result<bool> {
  using ResultT = Result<bool>;

  if (cursor_ + n > tiny_json_str_.size()) {
    return ResultT::Err(Error::From(FlatJson{}
                                        .Set("kind", "advance")
                                        .Set("message", "advance out of range")
                                        .Set("cursor", cursor_)
                                        .Set("n", n)
                                        .Take()));
  }

  cursor_ += n;
  return ResultT::Ok(true);
}

auto
kero::FlatJsonParser::Trim() noexcept -> void {
  while (cursor_ < tiny_json_str_.size() &&
         isspace(static_cast<int>(tiny_json_str_[cursor_]))) {
    ++cursor_;
  }
}
