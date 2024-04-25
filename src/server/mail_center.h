#ifndef SERVER_MAIL_CENTER_H
#define SERVER_MAIL_CENTER_H

#include <memory>
#include <thread>
#include <unordered_map>

#include "core/core.h"
#include "core/spsc_channel.h"

#include "common.h"

using MessageBody = std::unordered_map<std::string, std::string>;

struct Mail {
  std::string from;
  std::string to;
  MessageBody body;
};

struct MailBox final : private NonCopyable, Movable {
  Tx<Mail> tx;
  Rx<Mail> rx;

private:
  explicit MailBox(Tx<Mail> &&tx, Rx<Mail> &&rx) noexcept;

  friend class MailCenter;
};

class MailCenter final : private NonCopyable, NonMovable {
public:
  auto Shutdown() noexcept -> void;

  [[nodiscard]] auto Create(const std::string_view name) noexcept
      -> Result<MailBox>;
  [[nodiscard]] auto Delete(const std::string_view name) noexcept
      -> Result<Void>;

  static auto Global() noexcept -> MailCenter &;

private:
  explicit MailCenter(Tx<MessageBody> &&run_tx) noexcept;

  auto RunOnThread(Rx<MessageBody> &&run_rx) noexcept -> void;

  auto StartRunThread(Rx<MessageBody> &&run_rx) noexcept -> void;

  static auto RunThreadMain(MailCenter &mail_center,
                            Rx<MessageBody> &&run_rx) noexcept -> void;

  std::unordered_map<std::string, MailBox> mail_boxes_;
  std::mutex mutex_;
  Tx<MessageBody> run_tx_;
  std::thread run_thread_;
};

#endif // SERVER_MAIL_CENTER_H
