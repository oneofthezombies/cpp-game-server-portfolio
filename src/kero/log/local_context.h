#ifndef KERO_LOG_LOCAL_CONTEXT_H
#define KERO_LOG_LOCAL_CONTEXT_H

#include <memory>

#include "kero/core/result.h"
#include "kero/core/spsc_channel.h"
#include "kero/log/core.h"

namespace kero {

class LocalContext final {
 public:
  class Builder {
   public:
    Builder() noexcept = default;
    ~Builder() noexcept = default;
    CLASS_KIND_PINNABLE(Builder);

    auto
    Build() const noexcept -> Result<std::unique_ptr<LocalContext>>;
  };

  ~LocalContext() noexcept;
  CLASS_KIND_MOVABLE(LocalContext);

  auto
  SendLog(std::unique_ptr<kero::Log>&& log) const noexcept -> void;

 private:
  LocalContext(spsc::Tx<std::unique_ptr<kero::Log>>&& log_tx,
               std::string&& thread_id) noexcept;

  spsc::Tx<std::unique_ptr<kero::Log>> log_tx_;
  std::string thread_id_;
};

auto
GetLocalContext() -> std::unique_ptr<LocalContext>&;

}  // namespace kero

#endif  // KERO_LOG_LOCAL_CONTEXT_H
