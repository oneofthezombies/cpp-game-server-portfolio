#ifndef SERVER_CONTENTS_COMMON_H
#define SERVER_CONTENTS_COMMON_H

#include "server/engine/common.h"

namespace contents {

/**
 * Server contents symbols start from 2,000,000
 */
enum Symbol : int32_t {
  kServerContentsBegin = 2'000'000,
  // Add symbols after kBegin

  // Add symbols before kEnd
  kServerContentsEnd
};

auto operator<<(std::ostream &os, const Symbol symbol) noexcept
    -> std::ostream &;

using Void = engine::Void;
using Error = engine::Error;

template <typename T> using Result = engine::Result<T>;

} // namespace contents

#endif // SERVER_CONTENTS_COMMON_H
