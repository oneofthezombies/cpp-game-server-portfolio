#ifndef KERO_ENGINE_SERVICE_KIND_H
#define KERO_ENGINE_SERVICE_KIND_H

#include "kero/core/common.h"

namespace kero {

class Service;

using ServiceKindId = i64;
using ServiceKindName = std::string_view;

template <typename T>
concept IsServiceKind = std::is_base_of_v<Service, T> && requires {
  { T::kKindId } -> std::convertible_to<ServiceKindId>;
  { T::kKindName } -> std::convertible_to<ServiceKindName>;
};

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_KIND_H
