#ifndef KERO_ENGINE_ACTOR_SYSTEM_H
#define KERO_ENGINE_ACTOR_SYSTEM_H

#include <thread>
#include <unordered_map>

#include "kero/core/common.h"
#include "kero/core/dict.h"
#include "kero/core/result.h"
#include "kero/core/spsc_channel.h"
#include "kero/engine/actor_service.h"

namespace kero {

class ActorSystem final {
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
  DestroyMailBox(const std::string &name) noexcept -> bool;

 private:
  auto
  ValidateName(const std::string &name) const noexcept -> Result<Void>;

  static auto
  ThreadMain(ActorSystemPtr self) -> void;

  std::unordered_map<std::string, MailBox> mail_boxes_;
  std::mutex mutex_;
  spsc::Channel<Dict> run_channel_;
  std::thread run_thread_;

  static constexpr auto kMaxNameLength = 64;
};

}  // namespace kero

#endif  // KERO_ENGINE_ACTOR_SYSTEM_H
