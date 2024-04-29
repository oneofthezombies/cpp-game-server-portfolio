#include "tiny_json.h"

using namespace kero;

auto
kero::TinyJson::Stringify(const Dict &dict) noexcept -> std::string {
  std::string str;
  str += "{";

  for (auto it = dict.AsRaw().begin(); it != dict.AsRaw().end(); ++it) {
    str += "\"";
    str += it->first;
    str += "\": ";

    std::visit(
        [&str](const auto &value) {
          if constexpr (std::is_same_v<decltype(value), bool>) {
            str += value ? "true" : "false";
          } else if constexpr (std::is_same_v<decltype(value), double>) {
            str += std::to_string(value);
          } else if constexpr (std::is_same_v<decltype(value), std::string>) {
            str += "\"";
            for (const auto c : value) {
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
          }
        },
        it->second);

    if (std::next(it) != dict.AsRaw().end()) {
      str += ", ";
    }
  }

  str += "}";
  return str;
}

auto
kero::TinyJson::Parse(const std::string_view tiny_json_str,
                      ParseOptions &&options) noexcept -> Result<Dict> {
  tiny_json_str_ = tiny_json_str;
  cursor_ = 0;
  options_ = std::move(options);

  auto res = ParseObject();
  if (res.IsErr()) {
    return Result<Dict>::Err(Error::From(res.TakeErr()));
  }

  Trim();
  if (cursor_ < tiny_json_str_.size()) {
    return Result<Dict>::Err(
        Error::From(Dict{}
                        .Set("kind", std::string{"parse"})
                        .Set("message", std::string{"trailing characters"})
                        .Set("cursor", static_cast<double>(cursor_))
                        .Take()));
  }

  return Result<Dict>::Ok(res.TakeOk());
}

auto
kero::TinyJson::ParseObject() noexcept -> Result<Dict> {
  using ResultT = Result<Dict>;

  Trim();
  auto current = Current();
  if (current.IsErr()) {
    return ResultT::Err(Error::From(current.TakeErr()));
  }

  if (current.Ok() != '{') {
    return ResultT::Err(
        Error::From(Dict{}
                        .Set("kind", std::string{"object"})
                        .Set("message", std::string{"invalid object"})
                        .Set("cursor", static_cast<double>(cursor_))
                        .Take()));
  }

  if (auto res = Advance(); res.IsErr()) {
    return ResultT::Err(Error::From(res.TakeErr()));
  }

  Dict::Data data;
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

  return ResultT::Ok(Dict{std::move(data)});
}

auto
kero::TinyJson::ParseKey() noexcept -> Result<std::string> {
  using ResultT = Result<std::string>;

  auto key = ParseString();
  if (key.IsErr()) {
    return ResultT::Err(Error::From(key.TakeErr()));
  }

  return ResultT::Ok(key.TakeOk());
}

auto
kero::TinyJson::ParseValue() noexcept -> Result<DictValue> {
  using ResultT = Result<DictValue>;

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
        return ResultT::Ok(DictValue{res.TakeOk()});
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
        return ResultT::Ok(DictValue{res.TakeOk()});
      }
    }
    case 't':
    case 'f': {
      if (auto res = ParseBool(); res.IsErr()) {
        return ResultT::Err(Error::From(res.TakeErr()));
      } else {
        return ResultT::Ok(DictValue{res.TakeOk()});
      }
    }
    default: {
      return ResultT::Err(
          Error::From(Dict{}
                          .Set("kind", std::string{"value"})
                          .Set("message", std::string{"invalid value"})
                          .Set("cursor", static_cast<double>(cursor_))
                          .Take()));
    }
  }

  return ResultT::Err(
      Error::From(Dict{}
                      .Set("kind", std::string{"value"})
                      .Set("message", std::string{"invalid value"})
                      .Set("cursor", static_cast<double>(cursor_))
                      .Take()));
}

auto
kero::TinyJson::ParseString() noexcept -> Result<std::string> {
  using ResultT = Result<std::string>;

  Trim();
  auto current = Current();
  if (current.IsErr()) {
    return ResultT::Err(Error::From(current.TakeErr()));
  }

  if (current.Ok() != '"') {
    return ResultT::Err(
        Error::From(Dict{}
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
              Error::From(Dict{}
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
          Error::From(Dict{}
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
kero::TinyJson::ParseNumber() noexcept -> Result<double> {
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
        Error::From(Dict{}
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
          Error::From(Dict{}
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
          Error::From(Dict{}
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
kero::TinyJson::ParseBool() noexcept -> Result<bool> {
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
      Error::From(Dict{}
                      .Set("kind", std::string{"bool"})
                      .Set("message", std::string{"invalid bool"})
                      .Set("cursor", static_cast<double>(cursor_))
                      .Take()));
}

auto
kero::TinyJson::Current() const noexcept -> Result<char> {
  using ResultT = Result<char>;

  if (cursor_ >= tiny_json_str_.size()) {
    return ResultT::Err(
        Error::From(Dict{}
                        .Set("kind", std::string{"current"})
                        .Set("message", std::string{"current out of range"})
                        .Set("cursor", static_cast<double>(cursor_))
                        .Take()));
  }

  auto c = tiny_json_str_[cursor_];
  return ResultT::Ok(std::move(c));
}

auto
kero::TinyJson::Consume(const char c) noexcept -> Result<bool> {
  using ResultT = Result<bool>;

  if (cursor_ >= tiny_json_str_.size()) {
    return ResultT::Err(
        Error::From(Dict{}
                        .Set("kind", std::string{"consume"})
                        .Set("message", std::string{"consume out of range"})
                        .Set("cursor", static_cast<double>(cursor_))
                        .Take()));
  }

  if (tiny_json_str_[cursor_] != c) {
    return ResultT::Err(
        Error::From(Dict{}
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
kero::TinyJson::Advance(const size_t n) noexcept -> Result<bool> {
  using ResultT = Result<bool>;

  if (cursor_ + n > tiny_json_str_.size()) {
    return ResultT::Err(
        Error::From(Dict{}
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
kero::TinyJson::Trim() noexcept -> void {
  while (cursor_ < tiny_json_str_.size() &&
         isspace(static_cast<int>(tiny_json_str_[cursor_]))) {
    ++cursor_;
  }
}
