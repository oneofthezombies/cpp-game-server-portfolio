#ifndef KERO_ENGINE_ENGINE_H
#define KERO_ENGINE_ENGINE_H

#include "kero/core/common.h"
#include "kero/core/result.h"
#include "kero/engine/pin_object_system.h"
#include "kero/engine/service.h"

namespace kero {

class Context {
 public:
  explicit Context() noexcept;
  ~Context() noexcept;
  CLASS_KIND_PINNABLE(Context);

 private:
};

class ThreadRunner {
 public:
  class Builder {
   public:
    explicit Builder() noexcept = default;
    ~Builder() noexcept = default;

    [[nodiscard]] auto
    AddService(ServiceFactory&& service_factory) noexcept -> Builder&;

    [[nodiscard]] auto
    Build() const noexcept -> Result<Pin<ThreadRunner>>;

   private:
    std::vector<ServiceFactory> service_factories_;
  };

  explicit ThreadRunner(std::string&& name) noexcept;
  ~ThreadRunner() noexcept;

 private:
};

class Runner {
 public:
  class Builder {
   public:
  };

  explicit Runner(std::string&& name) noexcept;
  ~Runner() noexcept;

  auto
  AddService(ServiceFactory&& factory) -> Result<Void>;
};

class Engine {
 public:
  explicit Engine() noexcept;
  ~Engine() noexcept;
  CLASS_KIND_PINNABLE(Engine);

  [[nodiscard]] auto
  CreateThreadRunner(std::string&& name) -> Result<Pin<ThreadRunner>>;

  [[nodiscard]] auto
  CreateRunner(std::string&& name) -> Result<Pin<Runner>>;

  [[nodiscard]] static auto
  Global() -> Engine&;

 private:
  PinObjectSystem pin_object_system_;
};

}  // namespace kero

#endif  // KERO_ENGINE_ENGINE_H
