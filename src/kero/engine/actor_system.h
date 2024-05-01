#ifndef KERO_ENGINE_ACTOR_SYSTEM_H
#define KERO_ENGINE_ACTOR_SYSTEM_H

#include <thread>
#include <unordered_map>

#include "kero/core/common.h"
#include "kero/core/dict.h"
#include "kero/core/result.h"
#include "kero/core/spsc_channel.h"
#include "kero/engine/pin_object_system.h"

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
  std::string name;
  spsc::Tx<Mail> tx;
  spsc::Rx<Mail> rx;

  ~MailBox() noexcept = default;
  CLASS_KIND_MOVABLE(MailBox);

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
  CLASS_KIND_PINNABLE(ActorSystem);

  [[nodiscard]] auto
  CreateMailBox(const std::string &name) noexcept -> Result<MailBox>;

  [[nodiscard]] auto
  DestroyMailBox(const std::string &name) noexcept -> Result<Void>;

  [[nodiscard]] auto
  Run(spsc::Rx<Dict> &&rx) -> Result<Void>;

 private:
  [[nodiscard]] auto
  ValidateName(const std::string &name) const noexcept -> Result<Void>;

  std::unordered_map<std::string, MailBox> mail_boxes_;
  std::mutex mutex_;

  static constexpr auto kMaxNameLength = 64;
};

class ThreadActorSystem {
 public:
  explicit ThreadActorSystem(Pinned<ActorSystem> actor_system) noexcept;
  ~ThreadActorSystem() noexcept = default;

  [[nodiscard]] auto
  Start() noexcept -> Result<Void>;

  [[nodiscard]] auto
  Stop() noexcept -> Result<Void>;

 private:
  static auto
  ThreadMain(Pinned<ActorSystem> actor_system, spsc::Rx<Dict> &&rx) -> void;

  Pinned<ActorSystem> actor_system_;
  Owned<spsc::Tx<Dict>> tx_;
  std::thread thread_;
};

}  // namespace kero

#endif  // KERO_ENGINE_ACTOR_SYSTEM_H
