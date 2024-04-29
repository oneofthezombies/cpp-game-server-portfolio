#ifndef KERO_ENGINE_AGENT_H
#define KERO_ENGINE_AGENT_H

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "kero/core/dict.h"
#include "kero/service/service.h"

namespace kero {

class Agent {
 public:
  enum : Error::Code {
    kInterrupted = 1,
  };

  explicit Agent() noexcept = default;
  virtual ~Agent() noexcept = default;
  CLASS_KIND_MOVABLE(Agent);

  [[nodiscard]] auto
  Run() noexcept -> Result<Void>;

  auto
  Invoke(const std::string& event, const Dict& data) noexcept -> void;

  template <typename T>
    requires std::is_base_of_v<Service, T>
  [[nodiscard]] auto
  AddService(std::unique_ptr<T>&& service) noexcept -> bool {
    auto [it, inserted] =
        services_.try_emplace(service->GetKind(), std::move(service));
    return inserted;
  }

  [[nodiscard]] auto
  SubscribeEvent(const std::string& event,
                 const Service::Kind service_kind) noexcept -> bool;

  [[nodiscard]] auto
  UnsubscribeEvent(const std::string& event,
                   const Service::Kind service_kind) noexcept -> bool;

  [[nodiscard]] auto
  GetService(const Service::Kind service_kind) const noexcept
      -> OptionRef<Service&>;

  template <typename T>
    requires std::is_base_of_v<Service, T>
  [[nodiscard]] auto
  GetServiceAs(const Service::Kind service_kind) const noexcept
      -> OptionRef<T&> {
    auto service = GetService(service_kind);
    if (service.IsNone()) {
      return None;
    }

    return service.Unwrap().As<T>(service_kind);
  }

  [[nodiscard]] auto
  HasService(const Service::Kind service_kind) const noexcept -> bool;

  template <typename T>
    requires std::is_base_of_v<Service, T>
  [[nodiscard]] auto
  HasServiceIs(const Service::Kind service_kind) const noexcept -> bool {
    auto service = GetService(service_kind);
    if (service.IsNone()) {
      return false;
    }

    return service.Unwrap().Is<T>(service_kind);
  }

 private:
  auto
  ResolveDependencies() noexcept -> Result<Void>;

  auto
  CreateServices() noexcept -> Result<Void>;

  auto
  DestroyServices() noexcept -> void;

  auto
  UpdateServices() noexcept -> void;

  std::unordered_map<Service::Kind, ServicePtr> services_;
  std::unordered_map<std::string /* event */, std::unordered_set<Service::Kind>>
      events_;
};

class ThreadAgent final {
 public:
  explicit ThreadAgent() noexcept = default;
  ~ThreadAgent() noexcept;

  [[nodiscard]] auto
  Start(Agent&& agent) noexcept -> bool;

  [[nodiscard]] auto
  Stop() noexcept -> bool;

  [[nodiscard]] auto
  IsRunning() const noexcept -> bool;

 private:
  static auto
  ThreadMain(Agent&& agent) -> void;

  std::thread thread_;
};

}  // namespace kero

#endif  // KERO_ENGINE_AGENT_H
