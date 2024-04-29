#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <ostream>
#include <string>

#include "kero/core/args_scanner.h"
#include "kero/core/common.h"
#include "kero/core/error.h"
#include "kero/core/result.h"
#include "kero/core/utils.h"

enum Symbol : int32_t {
  kHelpRequested = 0,
  kIpArgNotFound,
  kIpValueNotFound,
  kPortArgNotFound,
  kPortParsingFailed,
  kPortValueNotFound,
  kUnknownArgument,
};

auto
operator<<(std::ostream &os, const Symbol symbol) -> std::ostream & {
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

using Error = kero::Error;

template <typename T>
using Result = kero::Result<T>;

auto
ParseArgs(int argc, char **argv) noexcept -> Result<Config> {
  using ResultT = Result<Config>;

  Config config;
  kero::ArgsScanner scanner = kero::ArgsScanner::FromArgs(argc, argv);

  // Skip the first argument which is the program name
  scanner.Eat();

  while (true) {
    auto current = scanner.Current();
    if (!current) {
      break;
    }

    const auto token = current.Unwrap();
    if (token == "--help") {
      return ResultT{Error::From(kHelpRequested)};
    }

    if (token == "--ip") {
      const auto next = scanner.Next();
      if (!next) {
        return ResultT{Error::From(kIpValueNotFound)};
      }

      config.ip = next.Unwrap();
      scanner.Eat();
      continue;
    }

    if (token == "--port") {
      const auto next = scanner.Next();
      if (!next) {
        return ResultT{Error::From(kPortValueNotFound)};
      }

      const auto next_token = next.Unwrap();
      auto result = kero::ParseNumberString<uint16_t>(next_token);
      if (result.IsErr()) {
        return ResultT::Err(Error::From(
            kPortParsingFailed,
            kero::Dict{}.Set("token", std::string{next_token}).Take()));
      }

      config.port = result.Ok();
      scanner.Eat();
      continue;
    }

    return ResultT::Err(
        Error::From(kUnknownArgument,
                    kero::Dict{}.Set("token", std::string{token}).Take()));
  }

  if (config.ip.empty()) {
    return ResultT{Error::From(kIpArgNotFound)};
  }

  if (config.port == Config::kUndefinedPort) {
    return ResultT{Error::From(kPortArgNotFound)};
  }

  return ResultT{std::move(config)};
}

auto
main(int argc, char **argv) noexcept -> int {
  auto options_res = ParseArgs(argc, argv);
  if (options_res.IsErr()) {
    const auto &error = options_res.Err();
    if (error.code == kHelpRequested) {
      std::cout << "Usage: client [--ip <ip>] [--port <port>] [--room-id "
                   "<room_id>]";
    } else if (error.code == kIpArgNotFound) {
      std::cout << "Error: --ip argument not found";
    } else if (error.code == kIpValueNotFound) {
      std::cout << "Error: --ip argument value not found";
    } else if (error.code == kPortArgNotFound) {
      std::cout << "Error: --port argument not found";
    } else if (error.code == kPortValueNotFound) {
      std::cout << "Error: --port argument value not found";
    } else if (error.code == kPortParsingFailed) {
      std::cout << "Error: port parsing failed: " << error;
    } else if (error.code == kUnknownArgument) {
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
  kero::Defer defer{[sock] { close(sock); }};

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(options.port);
  inet_pton(AF_INET, options.ip.data(), &server_addr.sin_addr);
  if (connect(sock, (sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    std::cerr << "Failed to connect to the server." << std::endl;
    return 1;
  }

  std::string buffer(4096, '\0');
  while (true) {
    auto read_size = read(sock, buffer.data(), buffer.size());
    if (read_size == -1) {
      std::cerr << "Failed to read from the server." << std::endl;
      return 1;
    }

    if (read_size == 0) {
      std::cout << "Server disconnected." << std::endl;
      break;
    }

    std::cout << "[DEBUG] Server: " << buffer << " (" << read_size << "bytes)"
              << std::endl;

    const auto message = core::TinyJson::Parse(buffer.substr(0, read_size));
    if (!message) {
      std::cout << "Failed to parse the message." << std::endl;
      continue;
    }

    auto kind_res = message->Get("kind");
    if (kind_res.IsErr()) {
      std::cout << "Failed to get the kind of the message." << std::endl;
      continue;
    }

    const auto kind = kind_res.Ok();
    if (kind == "connect") {
      const auto socket_id_res = message->GetAsNumber<SocketId>("socket_id");
      if (socket_id_res.IsErr()) {
        std::cout << "Failed to get the socket id." << std::endl;
        return 1;
      }

      const auto socket_id = socket_id_res.Ok();
      std::cout << "Connected to the server with socket id: " << socket_id
                << std::endl;
    } else if (kind == "battle_start") {
      const auto battle_id_res = message->GetAsNumber<BattleId>("battle_id");
      if (battle_id_res.IsErr()) {
        std::cout << "Failed to get the battle id." << std::endl;
        return 1;
      }

      const auto battle_id = battle_id_res.Ok();

      const auto opponent_socket_id_res =
          message->GetAsNumber<SocketId>("opponent_socket_id");
      if (opponent_socket_id_res.IsErr()) {
        std::cout << "Failed to get the opponent socket id." << std::endl;
        return 1;
      }

      const auto opponent_socket_id = opponent_socket_id_res.Ok();
      std::cout << "Battle started with battle id: " << battle_id
                << " and opponent socket id: " << opponent_socket_id
                << std::endl;

      std::string move;
      bool valid_move = false;
      while (!valid_move) {
        std::cout << "Please enter your move: ";
        std::cout << "Available moves: rock, paper, scissors, lizard, spock"
                  << std::endl;
        std::cin >> move;
        if (move != "rock" && move != "paper" && move != "scissors" &&
            move != "lizard" && move != "spock") {
          std::cout << "Invalid move." << std::endl;
          continue;
        }

        valid_move = true;
      }

      const auto data = core::TinyJson{}
                            .Set("kind", "battle_move")
                            .Set("move", move)
                            .ToString();
      auto count = write(sock, data.data(), data.size());
      if (count == -1) {
        std::cerr << "Failed to send the move to the server." << std::endl;
        return 1;
      }

      std::cout << "Move sent to the server. Waiting for the opponent's move."
                << std::endl;
    } else if ("battle_result") {
      const auto result_res = message->Get("result");
      if (result_res.IsErr()) {
        std::cout << "Failed to get the result." << std::endl;
        return 1;
      }

      const auto result = result_res.Ok();
      std::cout << "Battle result: " << result << std::endl;

      if (result == "win") {
        std::cout << "You won the battle." << std::endl;
      } else if (result == "lose") {
        std::cout << "You lost the battle." << std::endl;
      } else if (result == "draw") {
        std::cout << "The battle is a draw." << std::endl;
      } else {
        std::cout << "Unknown battle result: " << result << std::endl;
      }
    } else {
      std::cout << "Unknown message kind: " << kind << std::endl;
      continue;
    }
  }

  return 0;
}