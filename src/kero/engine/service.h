#ifndef KERO_SERVICE_SERVICE_H
#define KERO_SERVICE_SERVICE_H

#include <string>

#include "kero/core/common.h"
#include "kero/core/dict.h"
#include "kero/core/result.h"
#include "kero/engine/pin_object_system.h"

namespace kero {

class Context;
class Service;

template <typename T>
concept IsServiceKind = std::is_base_of_v<Service, T>;

class Service {
 public:
  using Kind = int64_t;
  using Dependencies = std::vector<Kind>;

  explicit Service(const Pin<Context> context,
                   const Kind kind,
                   const std::string&& kind_string,
                   Dependencies&& dependencies) noexcept;
  virtual ~Service() noexcept;
  CLASS_KIND_MOVABLE(Service);

  [[nodiscard]] auto
  GetKind() const noexcept -> Kind;

  [[nodiscard]] auto
  GetKindString() const noexcept -> const std::string&;

  [[nodiscard]] auto
  GetDependencies() const noexcept -> const Dependencies&;

  [[nodiscard]] auto
  Is(const Kind kind) const noexcept -> bool;

  [[nodiscard]] auto
  Is(const std::string& kind_string) const noexcept -> bool;

  template <IsServiceKind T>
  [[nodiscard]] auto
  As(const Kind kind) noexcept -> OptionRef<T&> {
    if (!Is(kind)) {
      return None;
    }

    return *static_cast<T*>(this);
  }

  template <IsServiceKind T>
  [[nodiscard]] auto
  As(const std::string& kind_string) noexcept -> OptionRef<T&> {
    if (!Is(kind_string)) {
      return None;
    }

    return *static_cast<T*>(this);
  }

  auto
  InvokeEvent(const std::string& event, const Dict& data) noexcept -> void;

  [[nodiscard]] auto
  SubscribeEvent(const std::string& event) -> Result<Void>;

  [[nodiscard]] auto
  UnsubscribeEvent(const std::string& event) -> Result<Void>;

  /**
   * Default implementation of the `OnCreate` method is empty.
   */
  [[nodiscard]] virtual auto
  OnCreate() noexcept -> Result<Void>;

  /**
   * Default implementation of the `OnDestroy` method is empty.
   */
  virtual auto
  OnDestroy(Agent& agent) noexcept -> void;

  /**
   * Default implementation of the `OnUpdate` method is empty.
   */
  virtual auto
  OnUpdate(Agent& agent) noexcept -> void;

  /**
   * Default implementation of the `OnEvent` method is empty.
   */
  virtual auto
  OnEvent(Agent& agent, const Event& event, const Dict& data) noexcept -> void;

 private:
  Dependencies dependencies_;
  std::string kind_string_;
  Kind kind_;
};

using ServicePtr = std::unique_ptr<Service>;
using ServiceFactory = std::function<Result<Service>(Pin<Context>)>;

}  // namespace kero

#endif  // KERO_SERVICE_SERVICE_H
