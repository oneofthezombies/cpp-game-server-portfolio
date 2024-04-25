#ifndef CORE_SPSC_CHANNEL_H
#define CORE_SPSC_CHANNEL_H

#include <atomic>
#include <memory>
#include <optional>

#include "core.h"

namespace core {

template <typename T>
  requires std::movable<T>
struct Node final : private NonCopyable, Movable {
  T data{};
  Node *next{};

  Node() = default;
  Node(T &&data) : data(std::move(data)) {}
};

template <typename T>
  requires std::movable<T>
class Queue final : private Copyable, Movable {
public:
  Queue() noexcept {
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

  auto Enqueue(T &&data) noexcept -> void {
    auto node = new Node<T>(std::move(data));
    auto tail = tail_.load(std::memory_order_acquire);
    tail->next = node;
    tail_.store(node, std::memory_order_release);
  }

  auto TryDequeue() noexcept -> std::optional<T> {
    auto head = head_.load(std::memory_order_acquire);
    auto head_next = head->next;
    if (head_next == nullptr) {
      return std::nullopt;
    }

    auto data = std::move(head_next->data);
    head_.store(head_next, std::memory_order_release);
    delete head;
    return data;
  }

  auto IsEmpty() const noexcept -> bool {
    return head_.load(std::memory_order_relaxed)->next == nullptr;
  }

private:
  std::atomic<Node<T> *> head_{};
  std::atomic<Node<T> *> tail_{};
};

template <typename T>
  requires std::movable<T>
class Tx final : private NonCopyable, Movable {
public:
  Tx(const std::shared_ptr<Queue<T>> &queue) : queue_{queue} {}

  auto Send(T &&value) const noexcept -> void {
    queue_->Enqueue(std::move(value));
  }

private:
  std::shared_ptr<Queue<T>> queue_;
};

template <typename T>
  requires std::movable<T>
class Rx final : private NonCopyable, Movable {
public:
  Rx(const std::shared_ptr<Queue<T>> &queue) : queue_{queue} {}

  auto TryReceive() const noexcept -> std::optional<T> {
    return queue_->TryDequeue();
  }

  auto IsEmpty() const noexcept -> bool { return queue_->IsEmpty(); }

private:
  std::shared_ptr<Queue<T>> queue_;
};

template <typename T>
  requires std::movable<T>
struct Channel final : private NonCopyable, Movable {
  class Builder final : private NonCopyable, NonMovable {
  public:
    auto Build() noexcept -> Channel<T> {
      auto queue = std::make_shared<Queue<T>>();
      auto tx = Tx<T>{queue};
      auto rx = Rx<T>{queue};
      return Channel<T>{std::move(tx), std::move(rx)};
    }
  };

  Tx<T> tx;
  Rx<T> rx;

private:
  Channel(Tx<T> &&tx, Rx<T> &&rx) : tx{std::move(tx)}, rx{std::move(rx)} {}
};

} // namespace core

#endif // CORE_SPSC_CHANNEL_H
