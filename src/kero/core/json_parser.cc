#include "json_parser.h"

#include <variant>

using namespace kero;

auto
kero::JsonParser::Stringify(const Json &json) noexcept -> Result<std::string> {
  using ResultT = Result<std::string>;

  std::string str;
  str += "{";

  for (auto it = json.AsRaw().begin(); it != json.AsRaw().end(); ++it) {
    const auto &key = it->first;
    const auto &value = it->second;

    str += "\"";
    str += key;
    str += "\":";

    if (std::holds_alternative<bool>(value)) {
      str += std::get<bool>(value) ? "true" : "false";
    } else if (std::holds_alternative<double>(value)) {
      str += std::to_string(std::get<double>(value));
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
      return ResultT::Err(Error::From(Json{}
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
kero::JsonParser::Parse(const std::string_view tiny_json_str,
                        ParseOptions &&options) noexcept -> Result<Json> {
  tiny_json_str_ = tiny_json_str;
  cursor_ = 0;
  options_ = std::move(options);

  auto res = ParseObject();
  if (res.IsErr()) {
    return Result<Json>::Err(Error::From(res.TakeErr()));
  }

  Trim();
  if (cursor_ < tiny_json_str_.size()) {
    return Result<Json>::Err(
        Error::From(Json{}
                        .Set("kind", std::string{"parse"})
                        .Set("message", std::string{"trailing characters"})
                        .Set("cursor", static_cast<double>(cursor_))
                        .Take()));
  }

  return Result<Json>::Ok(res.TakeOk());
}

auto
kero::JsonParser::ParseObject() noexcept -> Result<Json> {
  using ResultT = Result<Json>;

  Trim();
  auto current = Current();
  if (current.IsErr()) {
    return ResultT::Err(Error::From(current.TakeErr()));
  }

  if (current.Ok() != '{') {
    return ResultT::Err(
        Error::From(Json{}
                        .Set("kind", std::string{"object"})
                        .Set("message", std::string{"invalid object"})
                        .Set("cursor", static_cast<double>(cursor_))
                        .Take()));
  }

  if (auto res = Advance(); res.IsErr()) {
    return ResultT::Err(Error::From(res.TakeErr()));
  }

  Json::Data data;
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

  return ResultT::Ok(Json{std::move(data)});
}

auto
kero::JsonParser::ParseKey() noexcept -> Result<std::string> {
  using ResultT = Result<std::string>;

  auto key = ParseString();
  if (key.IsErr()) {
    return ResultT::Err(Error::From(key.TakeErr()));
  }

  return ResultT::Ok(key.TakeOk());
}

auto
kero::JsonParser::ParseValue() noexcept -> Result<Json::ValueStorage> {
  using ResultT = Result<Json::ValueStorage>;

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
        return ResultT::Ok(Json::ValueStorage{res.TakeOk()});
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
        return ResultT::Ok(Json::ValueStorage{res.TakeOk()});
      }
    }
    case 't':
    case 'f': {
      if (auto res = ParseBool(); res.IsErr()) {
        return ResultT::Err(Error::From(res.TakeErr()));
      } else {
        return ResultT::Ok(Json::ValueStorage{res.TakeOk()});
      }
    }
    default: {
      return ResultT::Err(
          Error::From(Json{}
                          .Set("kind", std::string{"value"})
                          .Set("message", std::string{"invalid value"})
                          .Set("cursor", static_cast<double>(cursor_))
                          .Take()));
    }
  }

  return ResultT::Err(
      Error::From(Json{}
                      .Set("kind", std::string{"value"})
                      .Set("message", std::string{"invalid value"})
                      .Set("cursor", static_cast<double>(cursor_))
                      .Take()));
}

auto
kero::JsonParser::ParseString() noexcept -> Result<std::string> {
  using ResultT = Result<std::string>;

  Trim();
  auto current = Current();
  if (current.IsErr()) {
    return ResultT::Err(Error::From(current.TakeErr()));
  }

  if (current.Ok() != '"') {
    return ResultT::Err(
        Error::From(Json{}
                        .Set("kind", std::string{"string"})
                        .Set("message", std::string{"invalid string"})
                        .Set("cursor", static_cast<double>(cursor_))
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
          return ResultT::Err(
              Error::From(Json{}
                              .Set("kind", std::string{"string"})
                              .Set("message", std::string{"invalid escape"})
                              .Set("cursor", static_cast<double>(cursor_))
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
      return ResultT::Err(
          Error::From(Json{}
                          .Set("kind", std::string{"string"})
                          .Set("message", std::string{"unterminated string"})
                          .Set("cursor", static_cast<double>(cursor_))
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
kero::JsonParser::ParseNumber() noexcept -> Result<double> {
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
    return ResultT::Err(
        Error::From(Json{}
                        .Set("kind", std::string{"number"})
                        .Set("message", std::string{"invalid number"})
                        .Set("cursor", static_cast<double>(cursor_))
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
      return ResultT::Err(
          Error::From(Json{}
                          .Set("kind", std::string{"number"})
                          .Set("message", std::string{"invalid number"})
                          .Set("cursor", static_cast<double>(cursor_))
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
      return ResultT::Err(
          Error::From(Json{}
                          .Set("kind", std::string{"number"})
                          .Set("message", std::string{"invalid number"})
                          .Set("cursor", static_cast<double>(cursor_))
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
kero::JsonParser::ParseBool() noexcept -> Result<bool> {
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

  return ResultT::Err(
      Error::From(Json{}
                      .Set("kind", std::string{"bool"})
                      .Set("message", std::string{"invalid bool"})
                      .Set("cursor", static_cast<double>(cursor_))
                      .Take()));
}

auto
kero::JsonParser::Current() const noexcept -> Result<char> {
  using ResultT = Result<char>;

  if (cursor_ >= tiny_json_str_.size()) {
    return ResultT::Err(
        Error::From(Json{}
                        .Set("kind", std::string{"current"})
                        .Set("message", std::string{"current out of range"})
                        .Set("cursor", static_cast<double>(cursor_))
                        .Take()));
  }

  auto c = tiny_json_str_[cursor_];
  return ResultT::Ok(std::move(c));
}

auto
kero::JsonParser::Consume(const char c) noexcept -> Result<bool> {
  using ResultT = Result<bool>;

  if (cursor_ >= tiny_json_str_.size()) {
    return ResultT::Err(
        Error::From(Json{}
                        .Set("kind", std::string{"consume"})
                        .Set("message", std::string{"consume out of range"})
                        .Set("cursor", static_cast<double>(cursor_))
                        .Take()));
  }

  if (tiny_json_str_[cursor_] != c) {
    return ResultT::Err(
        Error::From(Json{}
                        .Set("kind", std::string{"consume"})
                        .Set("message", std::string{"consume mismatch"})
                        .Set("cursor", static_cast<double>(cursor_))
                        .Set("expected", std::string{c})
                        .Set("actual", std::string{tiny_json_str_[cursor_]})
                        .Take()));
  }

  ++cursor_;
  return ResultT::Ok(true);
}

auto
kero::JsonParser::Advance(const size_t n) noexcept -> Result<bool> {
  using ResultT = Result<bool>;

  if (cursor_ + n > tiny_json_str_.size()) {
    return ResultT::Err(
        Error::From(Json{}
                        .Set("kind", std::string{"advance"})
                        .Set("message", std::string{"advance out of range"})
                        .Set("cursor", static_cast<double>(cursor_))
                        .Set("n", static_cast<double>(n))
                        .Take()));
  }

  cursor_ += n;
  return ResultT::Ok(true);
}

auto
kero::JsonParser::Trim() noexcept -> void {
  while (cursor_ < tiny_json_str_.size() &&
         isspace(static_cast<int>(tiny_json_str_[cursor_]))) {
    ++cursor_;
  }
}
