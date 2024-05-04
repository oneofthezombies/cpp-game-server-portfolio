#ifndef KERO_ENGINE_SERVICE_KIND_H
#define KERO_ENGINE_SERVICE_KIND_H

#include "kero/core/common.h"

namespace kero {

class Service;

using ServiceKindId = i64;
using ServiceKindName = std::string_view;

template <typename T>
concept IsServiceKind = std::is_base_of_v<Service, T> && requires {
  { T::kKindId } -> std::same_as<ServiceKindId>;
  { T::kKindName } -> std::same_as<ServiceKindName>;
};

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_KIND_H
