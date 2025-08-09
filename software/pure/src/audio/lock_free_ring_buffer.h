#pragma once

#include <atomic>
#include <memory>

template<typename T, size_t Size = 1024>
class LockFreeRingBuffer {
private:
  static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");
  static constexpr size_t MASK = Size - 1;
  
  alignas(64) std::atomic<size_t> head_{0};
  alignas(64) std::atomic<size_t> tail_{0};
  alignas(64) T buffer_[Size];

public:
  LockFreeRingBuffer() = default;
  
  LockFreeRingBuffer(const LockFreeRingBuffer&) = delete;
  LockFreeRingBuffer& operator=(const LockFreeRingBuffer&) = delete;
  
  bool try_push(const T& item) {
    const size_t current_tail = tail_.load(std::memory_order_relaxed);
    const size_t next_tail = (current_tail + 1) & MASK;
    
    if (next_tail == head_.load(std::memory_order_acquire)) {
      return false; // Buffer full
    }
    
    buffer_[current_tail] = item;
    tail_.store(next_tail, std::memory_order_release);
    return true;
  }
  
  bool try_pop(T& item) {
    const size_t current_head = head_.load(std::memory_order_relaxed);
    
    if (current_head == tail_.load(std::memory_order_acquire)) {
      return false; // Buffer empty
    }
    
    item = buffer_[current_head];
    head_.store((current_head + 1) & MASK, std::memory_order_release);
    return true;
  }
  
  bool empty() const {
    return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
  }
  
  bool full() const {
    const size_t current_tail = tail_.load(std::memory_order_acquire);
    const size_t next_tail = (current_tail + 1) & MASK;
    return next_tail == head_.load(std::memory_order_acquire);
  }
  
  size_t size() const {
    const size_t current_tail = tail_.load(std::memory_order_acquire);
    const size_t current_head = head_.load(std::memory_order_acquire);
    return (current_tail - current_head) & MASK;
  }
};