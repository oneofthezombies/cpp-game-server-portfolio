#include <cstring>
#include <functional>
#include <iostream>
#include <ostream>
#include <string>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "core/core.h"
#include "core/tiny_json.h"
#include "core/utils.h"

enum Symbol : int32_t {
  kHelpRequested = 0,
  kIpArgNotFound,
  kIpValueNotFound,
  kPortArgNotFound,
  kPortParsingFailed,
  kPortValueNotFound,
  kUnknownArgument,
};

auto operator<<(std::ostream &os, const Symbol symbol) -> std::ostream & {
  os << static_cast<std::underlying_type_t<Symbol>>(symbol);
  return os;
}

struct Config final {
  std::string ip;
  uint16_t port{kUndefinedPort};

  explicit Config() noexcept = default;
  ~Config() noexcept = default;
  CLASS_KIND_MOVABLE(Config);

  static constexpr uint16_t kUndefinedPort{0};
};

using Error = core::Error<Symbol>;

template <typename T> using Result = core::Result<T, Error>;

auto ParseArgs(core::Args &&args) noexcept -> Result<Config> {
  using ResultT = Result<Config>;

  Config config;
  core::Tokenizer tokenizer{std::move(args)};

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

      config.ip = *next;
      tokenizer.Eat();
      continue;
    }

    if (token == "--port") {
      const auto next = tokenizer.Next();
      if (!next.has_value()) {
        return ResultT{Error{Symbol::kPortValueNotFound}};
      }

      auto result = core::ParseNumberString<uint16_t>(*next);
      if (result.IsErr()) {
        return ResultT{
            Error{Symbol::kPortParsingFailed,
                  core::TinyJson{}.Set("error", result.Err()).IntoMap()}};
      }

      config.port = result.Ok();
      tokenizer.Eat();
      continue;
    }

    return ResultT{Error{Symbol::kUnknownArgument,
                         core::TinyJson{}.Set("token", token).IntoMap()}};
  }

  if (config.ip.empty()) {
    return ResultT{Error{Symbol::kIpArgNotFound}};
  }

  if (config.port == Config::kUndefinedPort) {
    return ResultT{Error{Symbol::kPortArgNotFound}};
  }

  return ResultT{std::move(config)};
}

auto main(int argc, char **argv) noexcept -> int {
  auto args = core::ParseArgcArgv(argc, argv);
  auto options_res = ParseArgs(std::move(args));
  if (options_res.IsErr()) {
    const auto &error = options_res.Err();
    if (error.code == Symbol::kHelpRequested) {
      std::cout << "Usage: client [--ip <ip>] [--port <port>] [--room-id "
                   "<room_id>]";
    } else if (error.code == Symbol::kIpArgNotFound) {
      std::cout << "Error: --ip argument not found";
    } else if (error.code == Symbol::kIpValueNotFound) {
      std::cout << "Error: --ip argument value not found";
    } else if (error.code == Symbol::kPortArgNotFound) {
      std::cout << "Error: --port argument not found";
    } else if (error.code == Symbol::kPortValueNotFound) {
      std::cout << "Error: --port argument value not found";
    } else if (error.code == Symbol::kPortParsingFailed) {
      std::cout << "Error: port parsing failed: " << error;
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
  core::Defer defer{[sock] { close(sock); }};

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(options.port);
  inet_pton(AF_INET, options.ip.data(), &server_addr.sin_addr);
  if (connect(sock, (sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    std::cerr << "Failed to connect to the server." << std::endl;
    return 1;
  }

  while (true) {
    char buffer[4096]{};
    auto read_size = read(sock, buffer, sizeof(buffer));
    if (read_size == -1) {
      std::cerr << "Failed to read from the server." << std::endl;
      return 1;
    }

    if (read_size == 0) {
      std::cout << "Server disconnected." << std::endl;
      break;
    }

    std::cout << "Server: " << buffer << " (" << read_size << " bytes)"
              << std::endl;
  }

  return 0;
}
