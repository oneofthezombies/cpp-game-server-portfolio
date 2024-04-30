#ifndef KERO_ENGINE_SERVICE_H
#define KERO_ENGINE_SERVICE_H

#include <string>

#include "kero/core/common.h"
#include "kero/core/dict.h"
#include "kero/core/result.h"
#include "kero/engine/pin_object_system.h"

namespace kero {

class RunnerContext;
class Service;

template <typename T>
concept IsServiceKind = std::is_base_of_v<Service, T>;

class Service {
 public:
  struct Kind {
    using Id = int64_t;
    using Name = std::string;

    Id id;
    Name name;
  };

  using Dependencies = std::vector<Kind::Name>;

  explicit Service(const Pin<RunnerContext> runner_context,
                   Kind&& kind,
                   Dependencies&& dependencies) noexcept;

  virtual ~Service() noexcept = default;
  CLASS_KIND_MOVABLE(Service);

  [[nodiscard]] auto
  GetKind() const noexcept -> const Kind&;

  [[nodiscard]] auto
  GetDependencies() const noexcept -> const Dependencies&;

  [[nodiscard]] auto
  Is(const Kind::Id kind_id) const noexcept -> bool;

  [[nodiscard]] auto
  Is(const Kind::Name& kind_name) const noexcept -> bool;

  template <IsServiceKind T>
  [[nodiscard]] auto
  As(const Kind::Id kind_id) noexcept -> OptionRef<T&> {
    if (!Is(kind_id)) {
      return None;
    }

    return *static_cast<T*>(this);
  }

  template <IsServiceKind T>
  [[nodiscard]] auto
  As(const Kind::Name& kind_name) noexcept -> OptionRef<T&> {
    if (!Is(kind_name)) {
      return None;
    }

    return *static_cast<T*>(this);
  }

  [[nodiscard]] auto
  SubscribeEvent(const std::string& event) -> Result<Void>;

  [[nodiscard]] auto
  UnsubscribeEvent(const std::string& event) -> Result<Void>;

  auto
  InvokeEvent(const std::string& event, const Dict& data) noexcept -> void;

  /**
   * Default implementation of the `OnCreate` method is noop.
   */
  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void>;

  /**
   * Default implementation of the `OnDestroy` method is noop.
   */
  virtual auto
  OnDestroy() noexcept -> void;

  /**
   * Default implementation of the `OnUpdate` method is noop.
   */
  virtual auto
  OnUpdate() noexcept -> void;

  /**
   * Default implementation of the `OnEvent` method is noop.
   */
  virtual auto
  OnEvent(const std::string& event, const Dict& data) noexcept -> void;

 private:
  Kind kind_;
  Dependencies dependencies_;
  Pin<RunnerContext> runner_context_;
};

using ServicePtr = std::unique_ptr<Service>;
using ServiceFactory = std::function<Result<ServicePtr>(Pin<RunnerContext>)>;

}  // namespace kero

#endif  // KERO_ENGINE_SERVICE_H
