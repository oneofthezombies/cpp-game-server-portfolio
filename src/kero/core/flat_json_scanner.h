#ifndef KERO_CORE_FLAT_JSON_SCANNER_H
#define KERO_CORE_FLAT_JSON_SCANNER_H

#include <string>

#include "kero/core/common.h"
#include "kero/core/option.h"

namespace kero {

class FlatJsonScanner {
 public:
  explicit FlatJsonScanner() noexcept = default;
  ~FlatJsonScanner() noexcept = default;
  KERO_CLASS_KIND_MOVABLE(FlatJsonScanner);

  auto
  Push(const std::string_view str) noexcept -> void;

  [[nodiscard]] auto
  Pop() noexcept -> Option<std::string>;

 private:
  std::string buffer_;
};

}  // namespace kero

#endif  // KERO_CORE_FLAT_JSON_SCANNER_H
