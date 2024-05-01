#ifndef KERO_ENGINE_RUNNER_H
#define KERO_ENGINE_RUNNER_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "kero/core/result.h"
#include "kero/engine/service.h"

namespace kero {

class Runner;

class RunnerContext {
 public:
  explicit RunnerContext(const Pin<Runner> runner) noexcept;
  ~RunnerContext() noexcept = default;
  CLASS_KIND_PINNABLE(RunnerContext);

  [[nodiscard]] auto
  SubscribeEvent(const std::string& event, const Service::Kind& kind)
      -> Result<Void>;

  [[nodiscard]] auto
  UnsubscribeEvent(const std::string& event, const Service::Kind& kind)
      -> Result<Void>;

  auto
  InvokeEvent(const std::string& event, const Dict& data) noexcept
      -> Result<Void>;

 private:
  Pin<Runner> runner_;
};

class Runner {
 public:
  using EventHandlerMapT =
      std::unordered_map<std::string /* event */,
                         std::unordered_set<Service::Kind>>;

  explicit Runner(std::string&& name) noexcept;
  ~Runner() noexcept = default;

  [[nodiscard]] auto
  Run() noexcept -> Result<Void>;

  [[nodiscard]] auto
  GetService(const Service::Kind::Id service_kind_id) const noexcept
      -> OptionRef<Service&>;

  [[nodiscard]] auto
  GetService(const Service::Kind::Name service_kind_name) const noexcept
      -> OptionRef<Service&>;

  template <IsServiceKind T>
  [[nodiscard]] auto
  GetServiceAs(const Service::Kind::Id service_kind_id) const noexcept
      -> OptionRef<T&> {
    auto service = GetService(service_kind_id);
    if (service.IsNone()) {
      return None;
    }

    return service.Unwrap().As<T>(service_kind_id);
  }

  template <IsServiceKind T>
  [[nodiscard]] auto
  GetServiceAs(const Service::Kind::Name service_kind_name) const noexcept
      -> OptionRef<T&> {
    auto service = GetService(service_kind_name);
    if (service.IsNone()) {
      return None;
    }

    return service.Unwrap().As<T>(service_kind_name);
  }

  [[nodiscard]] auto
  HasService(const Service::Kind::Id service_kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  HasService(const Service::Kind::Name service_kind_name) const noexcept
      -> bool;

  [[nodiscard]] auto
  HasServiceIs(const Service::Kind::Id service_kind_id) const noexcept -> bool {
    auto service = GetService(service_kind_id);
    if (service.IsNone()) {
      return false;
    }

    return service.Unwrap().Is(service_kind_id);
  }

  [[nodiscard]] auto
  HasServiceIs(const Service::Kind::Name service_kind_name) const noexcept
      -> bool {
    auto service = GetService(service_kind_name);
    if (service.IsNone()) {
      return false;
    }

    return service.Unwrap().Is(service_kind_name);
  }

  [[nodiscard]] auto
  SubscribeEvent(const std::string& event, const Service::Kind& kind)
      -> Result<Void>;

  [[nodiscard]] auto
  UnsubscribeEvent(const std::string& event, const Service::Kind& kind)
      -> Result<Void>;

  [[nodiscard]] auto
  InvokeEvent(const std::string& event, const Dict& data) noexcept
      -> Result<Void>;

 private:
  [[nodiscard]] auto
  ResolveDependencies() noexcept -> Result<Void>;

  [[nodiscard]] auto
  CreateServices() noexcept -> Result<Void>;

  auto
  DestroyServices() noexcept -> void;

  auto
  UpdateServices() noexcept -> void;

  ServiceMap service_map_;
  EventHandlerMapT event_handler_map_;
};

class ThreadRunner {
 public:
  explicit ThreadRunner(std::string&& name) noexcept;
  ~ThreadRunner() noexcept = default;
};

template <typename T>
concept IsRunnerKind =
    std::is_base_of_v<Runner, T> || std::is_base_of_v<ThreadRunner, T>;

template <IsRunnerKind T>
class RunnerBuilder {
 public:
  explicit RunnerBuilder() noexcept = default;
  ~RunnerBuilder() noexcept = default;

  [[nodiscard]] auto
  AddService(ServiceFactory&& service_factory) noexcept -> RunnerBuilder&;

  [[nodiscard]] auto
  Build() const noexcept -> Result<Pin<T>>;

 private:
  std::vector<ServiceFactory> service_factories_;
};

}  // namespace kero

#endif  // KERO_ENGINE_RUNNER_H
