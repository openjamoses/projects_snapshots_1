#pragma once

#include "utils.h"
#include "work_thread.h"

#include <optional.h>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <tuple>

// TODO: cleanup includes.

struct BaseThreadQueue {
  virtual bool IsEmpty() = 0;
  virtual ~BaseThreadQueue() = default;
};

// TODO Remove after migration to C++14
namespace {

template <size_t... Is>
struct index_sequence {};

template <size_t I, size_t... Is>
struct make_index_sequence {
  using type = typename make_index_sequence<I - 1, I - 1, Is...>::type;
};

template <size_t... Is>
struct make_index_sequence<0, Is...> {
  using type = index_sequence<Is...>;
};

}  // namespace

// std::lock accepts two or more arguments. We define an overload for one
// argument.
namespace std {
template <typename Lockable>
void lock(Lockable& l) {
  l.lock();
}
}  // namespace std

template <typename... Queue>
struct MultiQueueLock {
  MultiQueueLock(Queue... lockable) : tuple_{lockable...} { lock(); }
  ~MultiQueueLock() { unlock(); }
  void lock() {
    lock_impl(typename make_index_sequence<sizeof...(Queue)>::type{});
  }
  void unlock() {
    unlock_impl(typename make_index_sequence<sizeof...(Queue)>::type{});
  }

 private:
  template <size_t... Is>
  void lock_impl(index_sequence<Is...>) {
    std::lock(std::get<Is>(tuple_)->mutex_...);
  }

  template <size_t... Is>
  void unlock_impl(index_sequence<Is...>) {
    (void)std::initializer_list<int>{
        (std::get<Is>(tuple_)->mutex_.unlock(), 0)...};
  }

  std::tuple<Queue...> tuple_;
};

struct MultiQueueWaiter {
  std::condition_variable_any cv;

  static bool HasState(std::initializer_list<BaseThreadQueue*> queues) {
    for (BaseThreadQueue* queue : queues) {
      if (!queue->IsEmpty())
        return true;
    }
    return false;
  }

  template <typename... BaseThreadQueue>
  void Wait(BaseThreadQueue... queues) {
    MultiQueueLock<BaseThreadQueue...> l(queues...);
    while (!HasState({queues...}))
      cv.wait(l);
  }
};

// A threadsafe-queue. http://stackoverflow.com/a/16075550
template <class T>
struct ThreadedQueue : public BaseThreadQueue {
 public:
  ThreadedQueue() : total_count_(0) {
    owned_waiter_ = MakeUnique<MultiQueueWaiter>();
    waiter_ = owned_waiter_.get();
    owned_waiter1_ = MakeUnique<MultiQueueWaiter>();
    waiter1_ = owned_waiter1_.get();
  }

  // TODO remove waiter1 after split of on_indexed
  explicit ThreadedQueue(MultiQueueWaiter* waiter,
                         MultiQueueWaiter* waiter1 = nullptr)
      : total_count_(0), waiter_(waiter), waiter1_(waiter1) {}

  // Returns the number of elements in the queue. This is lock-free.
  size_t Size() const { return total_count_; }

  // Add an element to the queue.
  template <void (std::deque<T>::*push)(T&&)>
  void Push(T&& t, bool priority) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (priority)
      (priority_.*push)(std::move(t));
    else
      (queue_.*push)(std::move(t));
    ++total_count_;
    waiter_->cv.notify_one();
    if (waiter1_)
      waiter1_->cv.notify_one();
  }

  void PushFront(T&& t, bool priority = false) {
    Push<&std::deque<T>::push_front>(std::move(t), priority);
  }

  void PushBack(T&& t, bool priority = false) {
    Push<&std::deque<T>::push_back>(std::move(t), priority);
  }

  // Add a set of elements to the queue.
  void EnqueueAll(std::vector<T>&& elements, bool priority = false) {
    if (elements.empty())
      return;

    std::lock_guard<std::mutex> lock(mutex_);

    total_count_ += elements.size();

    for (T& element : elements) {
      if (priority)
        priority_.push_back(std::move(element));
      else
        queue_.push_back(std::move(element));
    }
    elements.clear();

    waiter_->cv.notify_all();
  }

  // Return all elements in the queue.
  std::vector<T> DequeueAll() {
    std::lock_guard<std::mutex> lock(mutex_);

    total_count_ = 0;

    std::vector<T> result;
    result.reserve(priority_.size() + queue_.size());
    while (!priority_.empty()) {
      result.emplace_back(std::move(priority_.front()));
      priority_.pop_front();
    }
    while (!queue_.empty()) {
      result.emplace_back(std::move(queue_.front()));
      queue_.pop_front();
    }

    return result;
  }

  // Returns true if the queue is empty. This is lock-free.
  bool IsEmpty() { return total_count_ == 0; }

  // Get the first element from the queue. Blocks until one is available.
  T Dequeue() {
    std::unique_lock<std::mutex> lock(mutex_);
    waiter_->cv.wait(lock,
                     [&]() { return !priority_.empty() || !queue_.empty(); });

    auto execute = [&](std::deque<T>* q) {
      auto val = std::move(q->front());
      q->pop_front();
      --total_count_;
      return std::move(val);
    };
    if (!priority_.empty())
      return execute(&priority_);
    return execute(&queue_);
  }

  // Get the first element from the queue without blocking. Returns a null
  // value if the queue is empty.
  optional<T> TryPopFrontHelper(int which) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto execute = [&](std::deque<T>* q) {
      auto val = std::move(q->front());
      q->pop_front();
      --total_count_;
      return std::move(val);
    };
    if (which & 2 && priority_.size())
      return execute(&priority_);
    if (which & 1 && queue_.size())
      return execute(&queue_);
    return nullopt;
  }

  optional<T> TryPopFront() {
    return TryPopFrontHelper(3);
  }

  optional<T> TryPopBack() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto execute = [&](std::deque<T>* q) {
      auto val = std::move(q->back());
      q->pop_back();
      --total_count_;
      return std::move(val);
    };
    // Reversed
    if (queue_.size())
      return execute(&queue_);
    if (priority_.size())
      return execute(&priority_);
    return nullopt;
  }

  optional<T> TryPopFrontLow() {
    return TryPopFrontHelper(1);
  }

  optional<T> TryPopFrontHigh() {
    return TryPopFrontHelper(2);
  }

  mutable std::mutex mutex_;

 private:
  std::atomic<int> total_count_;
  std::deque<T> priority_;
  std::deque<T> queue_;
  MultiQueueWaiter* waiter_;
  std::unique_ptr<MultiQueueWaiter> owned_waiter_;
  // TODO remove waiter1 after split of on_indexed
  MultiQueueWaiter* waiter1_;
  std::unique_ptr<MultiQueueWaiter> owned_waiter1_;
};
