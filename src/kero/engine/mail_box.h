#ifndef KERO_ENGINE_MAIL_BOX_H
#define KERO_ENGINE_MAIL_BOX_H

#include <string>

#include "kero/core/common.h"
#include "kero/core/dict.h"
#include "kero/core/spsc_channel.h"

namespace kero {

struct Mail final {
  std::string from;
  std::string to;
  std::string event;
  Dict body;

  explicit Mail() noexcept = default;
  explicit Mail(std::string &&from,
                std::string &&to,
                std::string &&event,
                Dict &&body) noexcept;
  ~Mail() noexcept = default;
  CLASS_KIND_MOVABLE(Mail);

  [[nodiscard]] auto
  Clone() const noexcept -> Mail;
};

struct MailBox final {
  spsc::Tx<Mail> tx;
  spsc::Rx<Mail> rx;

  ~MailBox() noexcept = default;
  CLASS_KIND_MOVABLE(MailBox);

 private:
  explicit MailBox(spsc::Tx<Mail> &&tx, spsc::Rx<Mail> &&rx) noexcept;

  friend class ActorSystem;
};

}  // namespace kero

#endif  // KERO_ENGINE_MAIL_BOX_H