#ifndef CORE_COMMON_H
#define CORE_COMMON_H

#include <cstdint>

/**
 * Common symbols start from 1,000,000
 */
enum Symbol : int32_t {
  kCommonBegin = 1'000'000,
  // Add symbols after kBegin

  kErrorPropagated,

  kTinyJsonKeyNotFound,

  // Add symbols before kEnd
  kCommonEnd
};

#endif  // CORE_COMMON_H
