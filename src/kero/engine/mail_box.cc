#include "mail_box.h"

using namespace kero;

kero::Mail::Mail(std::string &&from,
                 std::string &&to,
                 std::string &&event,
                 Dict &&body) noexcept
    : from{std::move(from)},
      to{std::move(to)},
      event{std::move(event)},
      body{std::move(body)} {}

auto
kero::Mail::Clone() const noexcept -> Mail {
  return Mail{std::string{from},
              std::string{to},
              std::string{event},
              body.Clone()};
}

kero::MailBox::MailBox(spsc::Tx<Mail> &&tx, spsc::Rx<Mail> &&rx) noexcept
    : tx{std::move(tx)}, rx{std::move(rx)} {}
