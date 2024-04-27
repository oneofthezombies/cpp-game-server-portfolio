#ifndef SERVER_ENGINE_MAIL_CENTER_H
#define SERVER_ENGINE_MAIL_CENTER_H

#include <thread>
#include <unordered_map>

#include "core/core.h"
#include "core/spsc_channel.h"
#include "core/tiny_json.h"

#include "common.h"

namespace engine {

using MailBody = core::TinyJson;

struct Mail final {
  std::string from;
  std::string to;
  MailBody body;

  explicit Mail() noexcept = default;
  explicit Mail(std::string &&from, std::string &&to, MailBody &&body) noexcept;
  ~Mail() noexcept = default;
  CLASS_KIND_MOVABLE(Mail);

  auto Clone() const noexcept -> Mail;
};

auto operator<<(std::ostream &os, const Mail &mail) noexcept -> std::ostream &;

struct MailBox final {
  core::Tx<Mail> tx;
  core::Rx<Mail> rx;

  ~MailBox() noexcept = default;
  CLASS_KIND_MOVABLE(MailBox);

private:
  explicit MailBox(core::Tx<Mail> &&tx, core::Rx<Mail> &&rx) noexcept;

  friend class MailCenter;
};

class MailCenter final {
public:
  ~MailCenter() noexcept = default;
  CLASS_KIND_PINNABLE(MailCenter);

  auto Shutdown() noexcept -> void;

  [[nodiscard]] auto Create(std::string &&name) noexcept -> Result<MailBox>;
  [[nodiscard]] auto Delete(std::string &&name) noexcept -> Result<Void>;

  static auto Global() noexcept -> MailCenter &;

private:
  explicit MailCenter(core::Tx<MailBody> &&run_tx) noexcept;

  auto ValidateName(const std::string_view name) const noexcept -> Result<Void>;

  auto RunOnThread(core::Rx<MailBody> &&run_rx) noexcept -> void;

  auto StartRunThread(core::Rx<MailBody> &&run_rx) noexcept -> void;

  static auto RunThreadMain(MailCenter &mail_center,
                            core::Rx<MailBody> &&run_rx) noexcept -> void;

  std::unordered_map<std::string, MailBox> mail_boxes_;
  std::mutex mutex_;
  core::Tx<MailBody> run_tx_;
  std::thread run_thread_;
};

} // namespace engine

#endif // SERVER_ENGINE_MAIL_CENTER_H
