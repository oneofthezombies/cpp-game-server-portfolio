#ifndef KERO_MIDDLEWARE_ACTOR_SERVICE_H
#define KERO_MIDDLEWARE_ACTOR_SERVICE_H

#include "kero/core/common.h"
#include "kero/engine/actor_system.h"
#include "kero/engine/service.h"
#include "kero/engine/service_factory.h"
#include "kero/engine/service_kind.h"
#include "runner_context.h"

namespace kero {

class Engine;

class ActorService final : public Service {
 public:
  virtual ~ActorService() noexcept override = default;
  CLASS_KIND_MOVABLE(ActorService);

  virtual auto
  OnUpdate() noexcept -> void override;

  [[nodiscard]] auto
  GetName() const noexcept -> const std::string &;

  auto
  SendMail(std::string &&to,
           std::string &&event,
           FlatJson &&body) noexcept -> void;

  auto
  BroadcastMail(std::string &&event, FlatJson &&body) noexcept -> void;

  [[nodiscard]] static auto
  GetKindId() noexcept -> ServiceKindId;

  [[nodiscard]] static auto
  GetKindName() noexcept -> ServiceKindName;

 private:
  explicit ActorService(const Pin<RunnerContext> runner_context,
                        std::string &&name,
                        MailBox &&mail_box) noexcept;

  MailBox mail_box_;
  std::string name_;

  friend class ActorServiceFactory;
};

class ActorServiceFactory final : public ServiceFactory {
 public:
  explicit ActorServiceFactory(const Own<Engine> &engine) noexcept;
  virtual ~ActorServiceFactory() noexcept override = default;
  CLASS_KIND_MOVABLE(ActorServiceFactory);

  [[nodiscard]] virtual auto
  Create(const Pin<RunnerContext> runner_context) noexcept
      -> Result<Own<Service>> override;

 private:
  Borrow<Engine> engine_;
};

}  // namespace kero

#endif  // KERO_MIDDLEWARE_ACTOR_SERVICE_H
