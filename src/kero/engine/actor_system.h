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
  explicit Actor(std::string &&name) noexcept;
  virtual ~Actor() noexcept override = default;
};

using ActorPtr = std::unique_ptr<Actor>;

class ActorSystem final {
 public:
  enum : int32_t {
    kEmptyNameNotAllowed = 1,
    kNameTooLong = 2,
  };

  static constexpr auto kMaxNameLength = 64;

  explicit ActorSystem() noexcept = default;
  ~ActorSystem() noexcept;
  CLASS_KIND_PINNABLE(ActorSystem);

  [[nodiscard]] auto
  Start() noexcept -> bool;

  [[nodiscard]] auto
  Stop() noexcept -> bool;

  [[nodiscard]] auto
  IsRunning() const noexcept -> bool;

  [[nodiscard]] auto
  CreateActor(std::string &&name) noexcept -> Result<ActorPtr>;

  [[nodiscard]] auto
  DeleteActor(const std::string &name) noexcept -> Result<Void>;

 private:
  auto
  ValidateName(const std::string &name) const noexcept -> Result<Void>;

  static auto
  ThreadMain() -> void;

  std::unordered_map<std::string, MailBox> mail_boxes_;
  std::mutex mutex_;
  std::thread thread_;
};

}  // namespace kero

#endif  // KERO_ENGINE_ACTOR_SYSTEM_H
