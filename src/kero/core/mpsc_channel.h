#ifndef KERO_CORE_MPSC_CHANNEL_H
#define KERO_CORE_MPSC_CHANNEL_H

#include <concepts>  // IWYU pragma: keep
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

#include "kero/core/common.h"
#include "kero/core/option.h"

namespace kero {
namespace mpsc {

template <typename T>
concept MoveOnly =                            //
    std::is_object_v<T> &&                    //
    std::is_nothrow_destructible_v<T> &&      //
    std::is_constructible_v<T, T> &&          //
    !std::is_constructible_v<T, const T> &&   //
    !std::is_constructible_v<T, T&> &&        //
    !std::is_constructible_v<T, const T&> &&  //
    std::swappable<T> &&                      //
    std::convertible_to<T, T> &&              //
    std::convertible_to<T, const T> &&        //
    !std::convertible_to<T, T&> &&            //
    std::convertible_to<T, const T&> &&       //
    std::assignable_from<T&, T> &&            //
    !std::assignable_from<T&, const T> &&     //
    !std::assignable_from<T&, T&> &&          //
    !std::assignable_from<T&, const T&>;

template <typename T>
  requires MoveOnly<T>
class Queue final {
 public:
  class Builder final {
   public:
    explicit Builder() noexcept = default;
    ~Builder() noexcept = default;
    KERO_CLASS_KIND_PINNABLE(Builder);

    auto
    Build() const noexcept -> Share<Queue<T>> {
      return Share<Queue<T>>{new Queue<T>{}};
    }
  };

  ~Queue() noexcept = default;
  KERO_CLASS_KIND_PINNABLE(Queue);

  auto
  Push(T&& item) noexcept -> void {
    std::lock_guard<std::mutex> lock{mutex_};
    queue_.push(std::move(item));
    condition_variable_.notify_one();
  }

  [[nodiscard]] auto
  Pop() noexcept -> T {
    std::unique_lock<std::mutex> lock{mutex_};
    condition_variable_.wait(lock, [this] { return !queue_.empty(); });
    auto item = std::move(queue_.front());
    queue_.pop();
    return item;
  }

  [[nodiscard]] auto
  TryPop() noexcept -> Option<T> {
    std::lock_guard<std::mutex> lock{mutex_};
    if (queue_.empty()) {
      return None;
    }
    auto item = std::move(queue_.front());
    queue_.pop();
    return item;
  }

  [[nodiscard]] auto
  TryPopAll() noexcept -> std::queue<T> {
    std::lock_guard<std::mutex> lock{mutex_};
    auto queue = std::move(queue_);
    return queue;
  }

 private:
  Queue() noexcept = default;

  std::queue<T> queue_{};
  std::mutex mutex_{};
  std::condition_variable condition_variable_{};
};

template <typename T>
  requires MoveOnly<T>
class Rx final {
 public:
  explicit Rx(const Share<Queue<T>>& queue) noexcept : queue_{queue} {}
  ~Rx() noexcept = default;
  KERO_CLASS_KIND_MOVABLE(Rx);

  [[nodiscard]] auto
  Receive() const noexcept -> T {
    return queue_->Pop();
  }

  [[nodiscard]] auto
  TryReceive() const noexcept -> Option<T> {
    return queue_->TryPop();
  }

  [[nodiscard]] auto
  TryReceiveAll() const noexcept -> std::queue<T> {
    return queue_->TryPopAll();
  }

 private:
  Share<Queue<T>> queue_;
};

template <typename T>
  requires MoveOnly<T>
class Tx final {
 public:
  explicit Tx(const Share<Queue<T>>& queue) noexcept : queue_{queue} {}
  ~Tx() noexcept = default;
  KERO_CLASS_KIND_MOVABLE(Tx);

  [[nodiscard]] auto
  Clone() const noexcept -> Tx<T> {
    return Tx<T>{queue_};
  }

  auto
  Send(T&& item) const noexcept -> void {
    queue_->Push(std::move(item));
  }

 private:
  Share<Queue<T>> queue_;
};

template <typename T>
  requires MoveOnly<T>
struct Channel final {
  class Builder final {
   public:
    Builder() noexcept = default;
    ~Builder() noexcept = default;
    KERO_CLASS_KIND_PINNABLE(Builder);

    [[nodiscard]] auto
    Build() const noexcept -> Channel<T> {
      auto queue = typename Queue<T>::Builder{}.Build();
      auto tx = Tx<T>{queue};
      auto rx = Rx<T>{queue};
      return Channel<T>{std::move(tx), std::move(rx)};
    }
  };

  Tx<T> tx;
  Rx<T> rx;

  ~Channel() noexcept = default;
  KERO_CLASS_KIND_MOVABLE(Channel);

 private:
  explicit Channel(Tx<T>&& tx, Rx<T>&& rx) noexcept
      : tx{std::move(tx)}, rx{std::move(rx)} {}
};

}  // namespace mpsc
}  // namespace kero

#endif  // KERO_CORE_MPSC_CHANNEL_H
