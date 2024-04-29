#ifndef KERO_LOG_TRANSPORT_H
#define KERO_LOG_TRANSPORT_H

#include "kero/log/core.h"

namespace kero {

class Transport {
 public:
  explicit Transport() noexcept;
  explicit Transport(const Level level) noexcept;
  virtual ~Transport() noexcept = default;
  CLASS_KIND_MOVABLE(Transport);

  auto
  SetLevel(const Level level) noexcept -> void;

  [[nodiscard]] auto
  GetLevel() const noexcept -> Level;

  virtual auto
  OnLog(const Log& log) noexcept -> void = 0;

 protected:
  Level level_;
};

class ConsolePlainTextTransport : public Transport {
 public:
  ConsolePlainTextTransport() noexcept = default;
  virtual ~ConsolePlainTextTransport() noexcept = default;
  CLASS_KIND_MOVABLE(ConsolePlainTextTransport);

  virtual auto
  OnLog(const Log& log) noexcept -> void override;
};

}  // namespace kero

#endif  // KERO_LOG_TRANSPORT_H
