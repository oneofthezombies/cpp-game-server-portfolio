#ifndef RPSLS_COMMON_H
#define RPSLS_COMMON_H

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

#endif  // RPSLS_COMMON_H