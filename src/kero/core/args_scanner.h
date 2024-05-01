#ifndef KERO_CORE_ARGS_SCANNER_H
#define KERO_CORE_ARGS_SCANNER_H

#include <string>
#include <vector>

#include "kero/core/common.h"
#include "kero/core/option.h"

namespace kero {

using Args = std::vector<std::string>;

class ArgsScanner final {
 public:
  explicit ArgsScanner(const Args &args) noexcept;
  explicit ArgsScanner(Args &&args) noexcept;
  ~ArgsScanner() noexcept = default;
  CLASS_KIND_MOVABLE(ArgsScanner);

  [[nodiscard]] auto
  Current() const noexcept -> OptionRef<const std::string &>;

  [[nodiscard]] auto
  Next() const noexcept -> OptionRef<const std::string &>;

  auto
  Eat() noexcept -> void;

 private:
  Args args_;
  size_t index_{};
};

}  // namespace kero

#endif  // KERO_CORE_ARGS_SCANNER_H
