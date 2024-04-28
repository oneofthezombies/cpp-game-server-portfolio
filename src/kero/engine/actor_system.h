#ifndef KERO_ENGINE_ACTOR_SYSTEM_H
#define KERO_ENGINE_ACTOR_SYSTEM_H

#include <thread>
#include <unordered_map>

#include "kero/core/channel.h"
#include "kero/core/common.h"
#include "kero/core/dict.h"
#include "kero/core/result.h"
#include "kero/engine/component.h"

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

class Actor final : public Component {
 public:
  explicit Actor(std::string &&name, MailBox &&mail_box) noexcept;
  virtual ~Actor() noexcept override = default;
  CLASS_KIND_MOVABLE(Actor);

 private:
  MailBox mail_box_;
};

using ActorPtr = std::unique_ptr<Actor>;

class ActorSystem final {
 public:
  enum : int32_t {
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
    Build() noexcept -> ActorSystem;
  };

  ~ActorSystem() noexcept;
  CLASS_KIND_PINNABLE(ActorSystem);

  [[nodiscard]] auto
  Start() noexcept -> Result<Void>;

  [[nodiscard]] auto
  Stop() noexcept -> bool;

  [[nodiscard]] auto
  IsRunning() const noexcept -> bool;

  [[nodiscard]] auto
  CreateActor(std::string &&name) noexcept -> Result<ActorPtr>;

  [[nodiscard]] auto
  DeleteMailBox(const std::string &name) noexcept -> bool;

 private:
  explicit ActorSystem(Tx<Dict> &&run_tx,
                       std::unique_ptr<Rx<Dict>> &&run_rx) noexcept;

  auto
  ValidateName(const std::string &name) const noexcept -> Result<Void>;

  static auto
  ThreadMain(ActorSystem &self, std::unique_ptr<Rx<Dict>> &&run_rx) -> void;

  std::unordered_map<std::string, MailBox> mail_boxes_;
  std::mutex mutex_;
  Tx<Dict> run_tx_;
  std::unique_ptr<Rx<Dict>> run_rx_;
  std::thread run_thread_;

  static constexpr auto kMaxNameLength = 64;
};

}  // namespace kero

#endif  // KERO_ENGINE_ACTOR_SYSTEM_H
