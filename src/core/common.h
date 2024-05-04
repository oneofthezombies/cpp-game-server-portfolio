#ifndef CORE_COMMON_H
#define CORE_COMMON_H

#include <cstdint>

namespace core {

/**
 * Common symbols start from 1,000,000
 */
enum Symbol : i32 {
  kCommonBegin = 1'000'000,
  // Add symbols after kBegin

  kErrorPropagated,

  kFlatJsonParserKeyNotFound,

  // Add symbols before kEnd
  kCommonEnd
};

using SocketId = u64;
using BattleId = u64;

}  // namespace core

#endif  // CORE_COMMON_H
