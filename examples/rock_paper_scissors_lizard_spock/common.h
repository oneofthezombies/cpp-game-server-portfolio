#ifndef RPSLS_COMMON_H
#define RPSLS_COMMON_H

#include "kero/engine/service_kind.h"

enum RpslsServiceKindId : kero::ServiceKindId {
  kServiceKindIdMatch = 1,
  kServiceKindIdBattle = 2,
};

struct EventBattleStart {
  static constexpr auto kEvent = "battle_start";
  static constexpr auto kBattleId = "battle_id";
  static constexpr auto kPlayer1 = "player1";
  static constexpr auto kPlayer2 = "player2";
};

#endif  // RPSLS_COMMON_H