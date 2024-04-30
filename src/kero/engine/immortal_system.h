#ifndef KERO_ENGINE_IMMORTAL_SYSTEM_H
#define KERO_ENGINE_IMMORTAL_SYSTEM_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <memory>
#include <unordered_map>

#include "kero/core/common.h"

namespace kero {

class Immortal {
 public:
  using Id = int64_t;

  explicit Immortal(const Id id) noexcept;
  virtual ~Immortal() noexcept = default;

  [[nodiscard]] auto
  GetId() const noexcept -> Id;

 protected:
  const Id id_;
};

template <typename T>
concept ImmortalKind = std::is_base_of_v<Immortal, T>;

template <ImmortalKind T>
class Im final {
 public:
  explicit Im(T* immortal) noexcept : immortal_{immortal} {}
  CLASS_KIND_COPYABLE(Im);

  [[nodiscard]] auto
  operator->() noexcept -> T* {
    return immortal_;
  }

  [[nodiscard]] auto
  operator->() const noexcept -> const T* {
    return immortal_;
  }

  [[nodiscard]] auto
  operator*() noexcept -> T& {
    return *immortal_;
  }

  [[nodiscard]] auto
  operator*() const noexcept -> const T& {
    return *immortal_;
  }

  [[nodiscard]] auto
  Unwrap() noexcept -> T& {
    return *immortal_;
  }

  [[nodiscard]] auto
  Unwrap() const noexcept -> const T& {
    return *immortal_;
  }

 private:
  T* immortal_;
};

class ImmortalSystem final {
 public:
  ~ImmortalSystem() noexcept = default;
  CLASS_KIND_PINNABLE(ImmortalSystem);

  template <ImmortalKind T>
  [[nodiscard]] auto
  Create(std::function<std::initializer_list<T>(Immortal::Id)>&& init) noexcept
      -> Im<T> {
    std::lock_guard lock{mutex_};
    const auto id = next_immortal_id_++;
    const auto immortal = std::make_unique<T>(init(id));
    const auto it = immortals_.emplace(id, std::move(immortal));
    return Im{it.first->second.get()};
  }

  auto
  test() -> void {
    const auto a = Create<Immortal>(
        [](Immortal::Id id) -> std::initializer_list<Immortal> {
          return std::initializer_list<Immortal>{id};
        });
  }

  [[nodiscard]] static auto
  Global() noexcept -> ImmortalSystem&;

 private:
  explicit ImmortalSystem() noexcept = default;

  std::unordered_map<Immortal::Id, std::unique_ptr<Immortal>> immortals_;
  std::mutex mutex_;
  std::atomic<Immortal::Id> next_immortal_id_{0};
};

}  // namespace kero

#endif  // KERO_ENGINE_IMMORTAL_SYSTEM_H
