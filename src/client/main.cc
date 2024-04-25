#include <cstring>
#include <functional>
#include <iostream>
#include <ostream>
#include <string>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "core.h"
#include "protocol.h"
#include "tiny_json.h"
#include "utils.h"

enum class Symbol : int32_t {
  kHelpRequested = 0,
  kIpArgNotFound,
  kIpValueNotFound,
  kPortArgNotFound,
  kPortParsingFailed,
  kPortValueNotFound,
  kRoomIdArgNotFound,
  kRoomIdValueNotFound,
  kUnknownArgument,
};

auto operator<<(std::ostream &os, const Symbol symbol) -> std::ostream & {
  os << static_cast<std::underlying_type_t<Symbol>>(symbol);
  return os;
}

struct ClientOptions final : private NonCopyable, Movable {
  std::string ip;
  uint16_t port{kUndefinedPort};
  std::string room_id;

  static constexpr uint16_t kUndefinedPort{0};
};

using Error = ErrorBase<Symbol>;

template <typename T> using Result = ResultBase<T, Error>;

auto ParseArgs(Args &&args) noexcept -> Result<ClientOptions> {
  using ResultT = Result<ClientOptions>;

  ClientOptions options;
  Tokenizer tokenizer{std::move(args)};

  // Skip the first argument which is the program name
  tokenizer.Eat();

  for (auto current = tokenizer.Current(); current.has_value();
       tokenizer.Eat(), current = tokenizer.Current()) {
    const auto token = *current;

    if (token == "--help") {
      return ResultT{Error{Symbol::kHelpRequested}};
    }

    if (token == "--ip") {
      const auto next = tokenizer.Next();
      if (!next.has_value()) {
        return ResultT{Error{Symbol::kIpValueNotFound}};
      }

      options.ip = *next;
      tokenizer.Eat();
      continue;
    }

    if (token == "--port") {
      const auto next = tokenizer.Next();
      if (!next.has_value()) {
        return ResultT{Error{Symbol::kPortValueNotFound}};
      }

      auto result = ParseNumberString<uint16_t>(*next);
      if (result.IsErr()) {
        return ResultT{Error{Symbol::kPortParsingFailed,
                             std::make_error_code(result.Err()).message()}};
      }

      options.port = result.Ok();
      tokenizer.Eat();
      continue;
    }

    if (token == "--room-id") {
      const auto next = tokenizer.Next();
      if (!next.has_value()) {
        return ResultT{Error{Symbol::kRoomIdValueNotFound}};
      }

      options.room_id = *next;
      tokenizer.Eat();
      continue;
    }

    return ResultT{Error{Symbol::kUnknownArgument, std::string{token}}};
  }

  if (options.ip.empty()) {
    return ResultT{Error{Symbol::kIpArgNotFound}};
  }

  if (options.port == ClientOptions::kUndefinedPort) {
    return ResultT{Error{Symbol::kPortArgNotFound}};
  }

  if (options.room_id.empty()) {
    return ResultT{Error{Symbol::kRoomIdArgNotFound}};
  }

  return ResultT{std::move(options)};
}

auto main(int argc, char **argv) noexcept -> int {
  auto args = ParseArgcArgv(argc, argv);
  auto options_res = ParseArgs(std::move(args));
  if (options_res.IsErr()) {
    const auto &error = options_res.Err();
    if (error.code == Symbol::kHelpRequested) {
      std::cout
          << "Usage: client [--ip <ip>] [--port <port>] [--room-id <room_id>]";
    } else if (error.code == Symbol::kIpArgNotFound) {
      std::cout << "Error: --ip argument not found";
    } else if (error.code == Symbol::kIpValueNotFound) {
      std::cout << "Error: --ip argument value not found";
    } else if (error.code == Symbol::kPortArgNotFound) {
      std::cout << "Error: --port argument not found";
    } else if (error.code == Symbol::kPortValueNotFound) {
      std::cout << "Error: --port argument value not found";
    } else if (error.code == Symbol::kPortParsingFailed) {
      std::cout << "Error: port parsing failed: " << error.message;
    } else if (error.code == Symbol::kRoomIdArgNotFound) {
      std::cout << "Error: --room-id argument not found";
    } else if (error.code == Symbol::kRoomIdValueNotFound) {
      std::cout << "Error: --room-id argument value not found";
    } else if (error.code == Symbol::kUnknownArgument) {
      std::cout << "Error: unknown argument";
    } else {
      std::cout << "Error: " << error;
    }

    std::cout << std::endl;
    return 1;
  }

  const auto &options = options_res.Ok();

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    std::cout << "Failed to create socket." << std::endl;
    return 1;
  }
  Defer defer{[sock] { close(sock); }};

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(options.port);
  inet_pton(AF_INET, options.ip.data(), &server_addr.sin_addr);
  if (connect(sock, (sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    std::cerr << "Failed to connect to the server." << std::endl;
    return 1;
  }

  const auto message = Message::BuildRaw(
      MessageKind::kRequest, 0,
      TinyJsonStringBuilder{}.Add("room_id", options.room_id).Build());
  if (send(sock, message.data(), message.size(), 0) == -1) {
    std::cerr << "Failed to send data." << std::endl;
    return 1;
  }

  char buffer[1024 * 8]{};
  ssize_t count = recv(sock, buffer, sizeof(buffer), 0);
  if (count == -1) {
    const char *error_message = strerror(errno);
    std::cerr << error_message << std::endl;
    std::cerr << "Failed to receive data." << std::endl;
    return 1;
  }

  std::cout << "Received message: " << buffer << std::endl;
  return 0;
}
