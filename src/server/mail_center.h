#ifndef SERVER_MAIL_CENTER_H
#define SERVER_MAIL_CENTER_H

#include <thread>
#include <unordered_map>

#include "core/core.h"
#include "core/spsc_channel.h"

#include "common.h"

using MailBody = std::unordered_map<std::string, std::string>;

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

struct MailBox final {
  Tx<Mail> tx;
  Rx<Mail> rx;

  ~MailBox() noexcept = default;
  CLASS_KIND_MOVABLE(MailBox);

private:
  explicit MailBox(Tx<Mail> &&tx, Rx<Mail> &&rx) noexcept;

  friend class MailCenter;
};

class MailCenter final {
public:
  ~MailCenter() noexcept = default;
  CLASS_KIND_PINNABLE(MailCenter);

  auto Shutdown() noexcept -> void;

  [[nodiscard]] auto Create(const std::string_view name) noexcept
      -> Result<MailBox>;
  [[nodiscard]] auto Delete(const std::string_view name) noexcept
      -> Result<Void>;

  static auto Global() noexcept -> MailCenter &;

private:
  explicit MailCenter(Tx<MailBody> &&run_tx) noexcept;

  auto ValidateName(const std::string_view name) const noexcept -> Result<Void>;

  auto RunOnThread(Rx<MailBody> &&run_rx) noexcept -> void;

  auto StartRunThread(Rx<MailBody> &&run_rx) noexcept -> void;

  static auto RunThreadMain(MailCenter &mail_center,
                            Rx<MailBody> &&run_rx) noexcept -> void;

  std::unordered_map<std::string, MailBox> mail_boxes_;
  std::mutex mutex_;
  Tx<MailBody> run_tx_;
  std::thread run_thread_;
};

#endif // SERVER_MAIL_CENTER_H
