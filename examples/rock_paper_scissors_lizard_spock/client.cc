#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <ostream>
#include <string>
#include <type_traits>

#include "common.h"
#include "kero/core/args_scanner.h"
#include "kero/core/common.h"
#include "kero/core/error.h"
#include "kero/core/flat_json.h"
#include "kero/core/flat_json_parser.h"
#include "kero/core/result.h"
#include "kero/core/utils.h"

/**
 * `using namespace kero` is used because it is an example program.
 */
using namespace kero;

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
  u16 port{kUndefinedPort};

  explicit Config() noexcept = default;
  ~Config() noexcept = default;
  KERO_CLASS_KIND_MOVABLE(Config);

  static constexpr u16 kUndefinedPort{0};
};

auto
ParseArgs(int argc, char **argv) noexcept -> Result<Config> {
  using ResultT = Result<Config>;

  Config config;
  kero::ArgsScanner scanner{kero::Args{argv, argv + argc}};

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
            kero::FlatJson{}.Set("token", std::string{next_token}).Take()));
      }

      config.port = result.Ok();
      scanner.Eat();
      scanner.Eat();
      continue;
    }

    return ResultT::Err(
        Error::From(kUnknownArgument,
                    kero::FlatJson{}.Set("token", std::string{token}).Take()));
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
      std::cout << "Usage: client [--ip <ip>] [--port <port>]";
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

  std::unordered_map<std::string, std::function<Result<Void>(const FlatJson &)>>
      event_handler_map;
  event_handler_map["connect"] = [](const FlatJson &data) -> Result<Void> {
    auto socket_id = data.TryGet<u64>("socket_id");
    if (!socket_id) {
      return Result<Void>::Err(
          FlatJson{}.Set("message", "Failed to get the socket id.").Take());
    }

    std::cout << "Connected to the server with socket_id: "
              << socket_id.Unwrap() << std::endl;
    return OkVoid();
  };

  event_handler_map["battle_start"] =
      [sock](const FlatJson &data) -> Result<Void> {
    const auto battle_id = data.TryGet<u64>("battle_id");
    if (!battle_id) {
      return Result<Void>::Err(
          FlatJson{}.Set("message", "Failed to get the battle id.").Take());
    }

    const auto opponent_socket_id = data.TryGet<u64>("opponent_socket_id");
    if (!opponent_socket_id) {
      return Result<Void>::Err(
          FlatJson{}
              .Set("message", "Failed to get the opponent socket id.")
              .Take());
    }

    std::cout << "Battle started with battle id: " << battle_id.Unwrap()
              << " and opponent socket id: " << opponent_socket_id.Unwrap()
              << std::endl;

    RpslsAction action{RpslsAction::kInvalid};
    while (action == RpslsAction::kInvalid) {
      std::cout << "Please enter your action: ";
      std::cout << "Available actions: rock, paper, scissors, lizard, spock"
                << std::endl;
      std::string action_str;
      std::cin >> action_str;
      action = StringToRpslsAction(action_str);
      if (action == RpslsAction::kInvalid) {
        std::cout << "Invalid action." << std::endl;
        continue;
      }
    }

    auto stringified_res = FlatJsonStringifier{}.Stringify(
        FlatJson{}
            .Set("__event", "battle_action")
            .Set("action",
                 static_cast<std::underlying_type_t<RpslsAction>>(action))
            .Take());
    if (stringified_res.IsErr()) {
      return Result<Void>::Err(
          FlatJson{}.Set("message", "Failed to stringify the action.").Take());
    }

    const auto stringified = stringified_res.TakeOk();
    auto count = send(sock, stringified.data(), stringified.size(), 0);
    if (count == -1) {
      return Result<Void>::Err(
          FlatJson{}
              .Set("message", "Failed to send the action to the server.")
              .Take());
    }

    std::cout << "Move sent to the server. Waiting for the opponent's action."
              << std::endl;
    return OkVoid();
  };

  event_handler_map["battle_result"] =
      [](const FlatJson &data) -> Result<Void> {
    const auto result_opt =
        data.TryGet<std::underlying_type_t<RpslsResult>>("result");
    if (!result_opt) {
      return Result<Void>::Err(
          FlatJson{}.Set("message", "Failed to get the result.").Take());
    }

    const auto result = RawToRpslsResult(result_opt.Unwrap());
    if (result == RpslsResult::kInvalid) {
      return Result<Void>::Err(
          FlatJson{}.Set("message", "Invalid result.").Take());
    }

    const auto result_str = RpslsResultToString(result);

    std::cout << "Battle result: " << result_str << std::endl;

    if (result == RpslsResult::kWin) {
      std::cout << "You won the battle." << std::endl;
    } else if (result == RpslsResult::kLose) {
      std::cout << "You lost the battle." << std::endl;
    } else {
      std::cout << "The battle is a draw." << std::endl;
    }

    return OkVoid();
  };

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

    const auto message =
        kero::FlatJsonParser{}.Parse(buffer.substr(0, read_size));
    if (message.IsErr()) {
      std::cout << "Failed to parse the message." << message.Err() << std::endl;
      continue;
    }

    auto event_opt = message.Ok().TryGet<std::string>("event");
    if (!event_opt) {
      std::cout << "Failed to get the event of the message." << std::endl;
      continue;
    }

    const auto &event = event_opt.Unwrap();

    auto found = event_handler_map.find(event);
    if (found == event_handler_map.end()) {
      std::cout << "Unknown event: " << event << std::endl;
      continue;
    }

    if (auto res = found->second(message.Ok()); res.IsErr()) {
      std::cout << "Failed to handle the event: " << res.Err() << std::endl;
    }
  }

  return 0;
}
