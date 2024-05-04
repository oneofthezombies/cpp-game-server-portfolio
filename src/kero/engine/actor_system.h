#ifndef KERO_ENGINE_ACTOR_SYSTEM_H
#define KERO_ENGINE_ACTOR_SYSTEM_H

#include <thread>
#include <unordered_map>

#include "kero/core/borrow.h"
#include "kero/core/common.h"
#include "kero/core/flat_json.h"
#include "kero/core/result.h"
#include "kero/core/spsc_channel.h"
#include "kero/engine/pin.h"

namespace kero {

struct Mail final {
  std::string from;
  std::string to;
  std::string event;
  FlatJson body;

  explicit Mail() noexcept = default;
  explicit Mail(std::string &&from,
                std::string &&to,
                std::string &&event,
                FlatJson &&body) noexcept;
  ~Mail() noexcept = default;
  KERO_CLASS_KIND_MOVABLE(Mail);

  [[nodiscard]] auto
  Clone() const noexcept -> Mail;
};

struct MailBox final {
  std::string name;
  spsc::Tx<Mail> tx;
  spsc::Rx<Mail> rx;

  ~MailBox() noexcept = default;
  KERO_CLASS_KIND_MOVABLE(MailBox);

 private:
  explicit MailBox(std::string &&name,
                   spsc::Tx<Mail> &&tx,
                   spsc::Rx<Mail> &&rx) noexcept;

  friend class ActorSystem;
};

class ActorSystem final {
 public:
  explicit ActorSystem() noexcept = default;
  ~ActorSystem() noexcept = default;
  KERO_CLASS_KIND_PINNABLE(ActorSystem);

  [[nodiscard]] auto
  CreateMailBox(const std::string &name) noexcept -> Result<MailBox>;

  [[nodiscard]] auto
  DestroyMailBox(const std::string &name) noexcept -> Result<Void>;

  [[nodiscard]] auto
  Run(spsc::Rx<FlatJson> &&rx) -> Result<Void>;

 private:
  [[nodiscard]] auto
  ValidateName(const std::string &name) const noexcept -> Result<Void>;

  std::unordered_map<std::string, MailBox> mail_box_map_{};
  std::mutex mutex_{};

  static constexpr auto kMaxNameLength = 64;
};

class ThreadActorSystem {
 public:
  explicit ThreadActorSystem(const Borrow<ActorSystem> actor_system) noexcept;
  ~ThreadActorSystem() noexcept = default;

  [[nodiscard]] auto
  Start() noexcept -> Result<Void>;

  [[nodiscard]] auto
  Stop() noexcept -> Result<Void>;

 private:
  static auto
  ThreadMain(const Borrow<ActorSystem> actor_system,
             spsc::Rx<FlatJson> &&rx) -> void;

  Borrow<ActorSystem> actor_system_;
  Own<spsc::Tx<FlatJson>> tx_;
  std::thread thread_;
};

}  // namespace kero

#endif  // KERO_ENGINE_ACTOR_SYSTEM_H
