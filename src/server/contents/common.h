#ifndef SERVER_CONTENTS_COMMON_H
#define SERVER_CONTENTS_COMMON_H

#include "core/core.h"

namespace contents {

/**
 * Server contents symbols start from 3,000,000
 */
enum Symbol : int32_t {
  kServerContentsBegin = 3'000'000,
  // Add symbols after kBegin

  // Add symbols before kEnd
  kServerContentsEnd
};

auto
operator<<(std::ostream &os, const Symbol symbol) noexcept -> std::ostream &;

using Void = core::Void;
using Error = core::Error;

template <typename T>
using Result = core::Result<T>;

using BattleId = uint64_t;

}  // namespace contents

#endif  // SERVER_CONTENTS_COMMON_H
