#ifndef KERO_CORE_CHANNEL_H
#define KERO_CORE_CHANNEL_H

#include <atomic>
#include <memory>

#include "kero/core/common.h"
#include "kero/core/option.h"

namespace kero {

template <typename T>
  requires std::movable<T>
struct Node final {
  T data{};
  Node *next{};

  explicit Node() noexcept = default;
  explicit Node(T &&data) noexcept : data(std::move(data)) {}
  ~Node() noexcept = default;
  CLASS_KIND_MOVABLE(Node);
};

template <typename T>
  requires std::movable<T>
class Queue final {
 public:
  explicit Queue() noexcept {
    auto node = new Node<T>{};
    head_.store(node, std::memory_order_relaxed);
    tail_.store(node, std::memory_order_relaxed);
  }

  ~Queue() noexcept {
    while (auto node = head_.load(std::memory_order_relaxed)) {
      head_.store(node->next, std::memory_order_relaxed);
      delete node;
    }
  }

  CLASS_KIND_PINNABLE(Queue);

  auto
  Enqueue(T &&data) noexcept -> void {
    auto node = new Node<T>(std::move(data));
    auto tail = tail_.load(std::memory_order_acquire);
    tail->next = node;
    tail_.store(node, std::memory_order_release);
  }

  [[nodiscard]] auto
  TryDequeue() noexcept -> Option<T> {
    auto head = head_.load(std::memory_order_acquire);
    auto head_next = head->next;
    if (head_next == nullptr) {
      return None;
    }

    auto data = std::move(head_next->data);
    head_.store(head_next, std::memory_order_release);
    delete head;
    return data;
  }

  [[nodiscard]] auto
  IsEmpty() const noexcept -> bool {
    return head_.load(std::memory_order_relaxed)->next == nullptr;
  }

 private:
  std::atomic<Node<T> *> head_{};
  std::atomic<Node<T> *> tail_{};
};

template <typename T>
  requires std::movable<T>
class Tx final {
 public:
  explicit Tx(const std::shared_ptr<Queue<T>> &queue) noexcept
      : queue_{queue} {}
  ~Tx() noexcept = default;
  CLASS_KIND_MOVABLE(Tx);

  auto
  Send(T &&value) const noexcept -> void {
    queue_->Enqueue(std::move(value));
  }

 private:
  std::shared_ptr<Queue<T>> queue_;
};

template <typename T>
  requires std::movable<T>
class Rx final {
 public:
  explicit Rx(const std::shared_ptr<Queue<T>> &queue) noexcept
      : queue_{queue} {}
  ~Rx() noexcept = default;
  CLASS_KIND_MOVABLE(Rx);

  [[nodiscard]] auto
  TryReceive() const noexcept -> Option<T> {
    return queue_->TryDequeue();
  }

  [[nodiscard]] auto
  IsEmpty() const noexcept -> bool {
    return queue_->IsEmpty();
  }

 private:
  std::shared_ptr<Queue<T>> queue_;
};

template <typename T>
  requires std::movable<T>
struct Channel final {
  class Builder final {
   public:
    explicit Builder() noexcept = default;
    ~Builder() noexcept = default;
    CLASS_KIND_PINNABLE(Builder);

    [[nodiscard]] auto
    Build() noexcept -> Channel<T> {
      auto queue = std::make_shared<Queue<T>>();
      auto tx = Tx<T>{queue};
      auto rx = Rx<T>{queue};
      return Channel<T>{std::move(tx), std::move(rx)};
    }
  };

  Tx<T> tx;
  Rx<T> rx;

  ~Channel() noexcept = default;
  CLASS_KIND_MOVABLE(Channel);

 private:
  Channel(Tx<T> &&tx, Rx<T> &&rx) noexcept
      : tx{std::move(tx)}, rx{std::move(rx)} {}
};

}  // namespace kero

#endif  // KERO_CORE_CHANNEL_H
