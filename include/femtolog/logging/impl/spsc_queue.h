// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <new>

#include "femtolog/base/check.h"
#include "femtolog/base/memory_util.h"

namespace femtolog::logging {

enum class SpscQueueStatus : uint8_t {
  kOk = 0,
  kUninitialized = 1,
  kUnderflow = 2,
  kOverflow = 3,
  kSizeIsZero = 4,
};

class SpscQueue {
 public:
  SpscQueue();
  ~SpscQueue() = default;

  SpscQueue(const SpscQueue&) = delete;
  SpscQueue& operator=(const SpscQueue&) = delete;

  SpscQueue(SpscQueue&&) noexcept = delete;
  SpscQueue& operator=(SpscQueue&&) noexcept = delete;

  void reserve(size_t capacity_bytes);

  template <typename T>
  inline SpscQueueStatus enqueue_bytes(const T* data) noexcept {
    return enqueue_bytes(reinterpret_cast<const void*>(data), sizeof(T));
  }
  SpscQueueStatus enqueue_bytes(const void* data_ptr,
                                size_t data_size) noexcept;

  template <typename T>
  inline SpscQueueStatus dequeue_bytes(T* data) noexcept {
    return dequeue_bytes(reinterpret_cast<void*>(data), sizeof(T));
  }
  SpscQueueStatus dequeue_bytes(void* data_ptr, size_t data_size) noexcept;

  template <typename T>
  inline SpscQueueStatus peek_bytes(T* data) const noexcept {
    return peek_bytes(reinterpret_cast<void*>(data), sizeof(T));
  }
  SpscQueueStatus peek_bytes(void* data_ptr, size_t data_size) const noexcept;

  [[nodiscard]] inline bool empty() const noexcept {
    const size_t head = head_cached_;
    const size_t tail = tail_idx_.load(std::memory_order_relaxed);
    return head == tail;
  }

  [[nodiscard]] inline size_t size() const noexcept {
    FEMTOLOG_DCHECK(buffer_);
    const size_t head = head_cached_;
    const size_t tail = tail_idx_.load(std::memory_order_relaxed);
    return tail - head;
  }

  [[nodiscard]] inline size_t capacity() const noexcept { return capacity_; }

  [[nodiscard]] inline size_t available_space() const noexcept {
    FEMTOLOG_DCHECK(buffer_);
    return capacity_ - size();
  }

  // Bulk operations for better performance
  SpscQueueStatus enqueue_bulk(const void* const* data_ptrs,
                               const size_t* data_sizes,
                               size_t count) noexcept;

  SpscQueueStatus dequeue_bulk(void* const* data_ptrs,
                               const size_t* data_sizes,
                               size_t count) noexcept;

  static constexpr size_t kBatchSize = 32;

 private:
  [[nodiscard]] static constexpr size_t next_power_of_2(size_t n) noexcept;

  // Cache line aligned buffer
  alignas(base::kCacheSize) std::byte* buffer_;
  size_t capacity_ = 0;
  size_t mask_ = 0;

  // Producer side (write-mostly, separate cache line)
  alignas(base::kCacheSize) std::atomic<size_t> tail_idx_ = 0;
  mutable size_t tail_cached_ = 0;
  // Cached snapshot of head for producer
  mutable size_t head_cached_snapshot_ = 0;

  // Consumer side (read-mostly, separate cache line)
  alignas(base::kCacheSize) std::atomic<size_t> head_idx_ = 0;
  mutable size_t head_cached_ = 0;
  // Cached snapshot of tail for consumer
  mutable size_t tail_cached_snapshot_ = 0;

  // Buffer management
  alignas(base::kCacheSize)
      std::unique_ptr<std::byte[], base::AlignedDeleter> buffer_deleter_ =
          nullptr;
  size_t allocation_size_ = 0;
};

// static
constexpr size_t SpscQueue::next_power_of_2(size_t n) noexcept {
  if (n <= 1) {
    return 2;
  }
  if ((n & (n - 1)) == 0) {
    return n;
  }

  // Use builtin count loading zeros for better performance
  return size_t(1) << (64 - __builtin_clzll(n - 1));
}

}  // namespace femtolog::logging
