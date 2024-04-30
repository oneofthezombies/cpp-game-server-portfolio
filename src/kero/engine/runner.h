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
  InvokeEvent(const std::string& event, const Dict& data) noexcept -> void;

 private:
  Pin<Runner> runner_;
};

class Runner {
 public:
  explicit Runner(std::string&& name) noexcept;
  ~Runner() noexcept = default;

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
  std::unordered_map<Service::Kind::Id, ServicePtr> services_;
  std::unordered_map<std::string /* event */, std::unordered_set<Service::Kind>>
      events_;
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
