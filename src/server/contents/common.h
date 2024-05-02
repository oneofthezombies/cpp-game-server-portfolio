#ifndef SERVER_CONTENTS_COMMON_H
#define SERVER_CONTENTS_COMMON_H

#include "core/common.h"
#include "core/core.h"

namespace contents {

/**
 * Server contents symbols start from 3,000,000
 */
enum Symbol : i32 {
  kServerContentsBegin = 3'000'000,
  // Add symbols after kBegin

  kLobbyMailGetSocketIdFailed,

  kBattleBattleIdNotFound,
  kBattleBattleStateNotFound,
  kBattleSocketIdNotFound,
  kBattleMoveNotFound,
  kBattleSocketMoveAlreadySet,
  kBattleMoveLogicError,

  // Add symbols before kEnd
  kServerContentsEnd
};

auto
operator<<(std::ostream &os, const Symbol symbol) noexcept -> std::ostream &;

using Void = core::Void;
using Error = core::Error;

template <typename T>
using Result = core::Result<T>;

using BattleId = core::BattleId;
using SocketId = core::SocketId;

}  // namespace contents

#endif  // SERVER_CONTENTS_COMMON_H
