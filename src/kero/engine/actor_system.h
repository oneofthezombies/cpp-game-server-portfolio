#ifndef KERO_ENGINE_ACTOR_SYSTEM_H
#define KERO_ENGINE_ACTOR_SYSTEM_H

#include <thread>
#include <unordered_map>

#include "kero/core/channel.h"
#include "kero/core/common.h"
#include "kero/core/dict.h"
#include "kero/core/result.h"
#include "kero/engine/service.h"

namespace kero {

struct Mail final {
  std::string from;
  std::string to;
  Dict body;

  explicit Mail() noexcept = default;
  explicit Mail(std::string &&from, std::string &&to, Dict &&body) noexcept;
  ~Mail() noexcept = default;
  CLASS_KIND_MOVABLE(Mail);

  [[nodiscard]] auto
  Clone() const noexcept -> Mail;
};

struct MailBox final {
  Tx<Mail> tx;
  Rx<Mail> rx;

  ~MailBox() noexcept = default;
  CLASS_KIND_MOVABLE(MailBox);

 private:
  explicit MailBox(Tx<Mail> &&tx, Rx<Mail> &&rx) noexcept;

  friend class ActorSystem;
};

class ActorService final : public Service {
 public:
  explicit ActorService(std::string &&name, MailBox &&mail_box) noexcept;
  virtual ~ActorService() noexcept override = default;
  CLASS_KIND_MOVABLE(ActorService);

  [[nodiscard]] virtual auto
  OnCreate(Agent &agent) noexcept -> Result<Void> override;

  virtual auto
  OnUpdate(Agent &agent) noexcept -> void override;

  [[nodiscard]] auto
  GetName() const noexcept -> const std::string &;

  auto
  SendMail(std::string &&to, Dict &&body) noexcept -> void;

 private:
  MailBox mail_box_;
  std::string name_;
};

using ActorServicePtr = std::unique_ptr<ActorService>;

class ActorSystem;

using ActorSystemPtr = std::shared_ptr<ActorSystem>;

class ActorSystem final : public std::enable_shared_from_this<ActorSystem> {
 public:
  enum : Error::Code {
    kEmptyNameNotAllowed = 1,
    kNameTooLong = 2,
    kMailBoxNameAlreadyExists = 3,
    kMailBoxNameNotFound = 4,
    kReservedNameNotAllowed = 5,
    kAlreadyRunning = 6,
    kMultipleStartNotAllowed,
  };

  class Builder final {
   public:
    explicit Builder() noexcept = default;
    ~Builder() noexcept = default;
    CLASS_KIND_PINNABLE(Builder);

    [[nodiscard]] auto
    Build() noexcept -> ActorSystemPtr;
  };

  explicit ActorSystem() noexcept;
  ~ActorSystem() noexcept;
  CLASS_KIND_PINNABLE(ActorSystem);

  [[nodiscard]] auto
  Start() noexcept -> Result<Void>;

  [[nodiscard]] auto
  Stop() noexcept -> bool;

  [[nodiscard]] auto
  IsRunning() const noexcept -> bool;

  [[nodiscard]] auto
  CreateActorService(std::string &&name) noexcept -> Result<ActorServicePtr>;

  [[nodiscard]] auto
  DeleteMailBox(const std::string &name) noexcept -> bool;

 private:
  auto
  ValidateName(const std::string &name) const noexcept -> Result<Void>;

  static auto
  ThreadMain(ActorSystemPtr self) -> void;

  std::unordered_map<std::string, MailBox> mail_boxes_;
  std::mutex mutex_;
  Channel<Dict> run_channel_;
  std::thread run_thread_;

  static constexpr auto kMaxNameLength = 64;
};

}  // namespace kero

#endif  // KERO_ENGINE_ACTOR_SYSTEM_H
