#ifndef KERO_ENGINE_SERVICE_KIND_H
#define KERO_ENGINE_SERVICE_KIND_H

#include <string>

namespace kero {

class Service;

template <typename T>
concept IsServiceKind = std::is_base_of_v<Service, T>;

struct ServiceKind {
  using Id = int64_t;
  using Name = std::string;

  Id id{};
  Name name{};
};

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_KIND_H