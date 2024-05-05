#ifndef RPSLS_COMMON_H
#define RPSLS_COMMON_H

#include <type_traits>

#include "kero/engine/service_kind.h"
#include "kero/middleware/common.h"

enum : kero::ServiceKindId {
  kServiceKindId_RpslsBegin = kero::kServiceKindId_MiddlewareEnd,
  kServiceKindId_Match,
  kServiceKindId_Battle,
  kServiceKindId_RpslsEnd,
};

struct EventBattleSocketCount {
  static constexpr auto kEvent = "battle_socket_count";
  static constexpr auto kName = "name";
  static constexpr auto kCount = "count";
};

struct EventBattleStart {
  static constexpr auto kEvent = "battle_start";
  static constexpr auto kBattleId = "battle_id";
  static constexpr auto kPlayer1SocketId = "player1_socket_id";
  static constexpr auto kPlayer2SocketId = "player2_socket_id";
};

struct EventBattleAction {
  static constexpr auto kEvent = "battle_action";
  static constexpr auto kSocketId = "__socket_id";
  static constexpr auto kAction = "action";
};

/**
 * rock -> lizard -> spock -> scissors -> paper -> rock
 * each defeats next(1) and next(3) in the sequence
 */
enum class RpslsAction : kero::i32 {
  kInvalid = 0,
  kRock,
  kPaper,
  kScissors,
  kLizard,
  kSpock,
};

inline auto
StringToRpslsAction(const std::string& action) noexcept -> RpslsAction {
  if (action == "rock") {
    return RpslsAction::kRock;
  } else if (action == "paper") {
    return RpslsAction::kPaper;
  } else if (action == "scissors") {
    return RpslsAction::kScissors;
  } else if (action == "lizard") {
    return RpslsAction::kLizard;
  } else if (action == "spock") {
    return RpslsAction::kSpock;
  }

  return RpslsAction::kInvalid;
}

inline auto
RawToRpslsAction(const std::underlying_type_t<RpslsAction> action) noexcept
    -> RpslsAction {
  if (action < static_cast<std::underlying_type_t<RpslsAction>>(
                   RpslsAction::kInvalid) ||
      action > static_cast<std::underlying_type_t<RpslsAction>>(
                   RpslsAction::kSpock)) {
    return RpslsAction::kInvalid;
  }

  return static_cast<RpslsAction>(action);
}

enum class RpslsResult : kero::i32 {
  kInvalid = 0,
  kWin,
  kLose,
  kDraw,
};

inline auto
StringToRpslsResult(const std::string& result) noexcept -> RpslsResult {
  if (result == "win") {
    return RpslsResult::kWin;
  } else if (result == "lose") {
    return RpslsResult::kLose;
  } else if (result == "draw") {
    return RpslsResult::kDraw;
  }

  return RpslsResult::kInvalid;
}

inline auto
RpslsResultToString(const RpslsResult result) noexcept -> std::string {
  switch (result) {
    case RpslsResult::kWin:
      return "win";
    case RpslsResult::kLose:
      return "lose";
    case RpslsResult::kDraw:
      return "draw";
    default:
      return "invalid";
  }
}

inline auto
RawToRpslsResult(const std::underlying_type_t<RpslsResult> result) noexcept
    -> RpslsResult {
  if (result < static_cast<std::underlying_type_t<RpslsResult>>(
                   RpslsResult::kInvalid) ||
      result > static_cast<std::underlying_type_t<RpslsResult>>(
                   RpslsResult::kDraw)) {
    return RpslsResult::kInvalid;
  }

  return static_cast<RpslsResult>(result);
}

#endif  // RPSLS_COMMON_H
