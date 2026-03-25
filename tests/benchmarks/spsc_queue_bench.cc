// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "catch2/benchmark/catch_benchmark.hpp"
#include "catch2/catch_test_macros.hpp"
#include "logging/impl/spsc_queue.h"

namespace femtolog::logging {

namespace {

constexpr size_t kDefaultQueueCapacity = 1024 * 4;       // 4KiB
constexpr size_t kMediumQueueCapacity = 1024 * 64;       // 64KiB
constexpr size_t kLargeQueueCapacity = 1024 * 1024 * 2;  // 2MiB

// Helper to simulate Google Benchmark's Range behavior
std::vector<int> generate_range(int start, int end) {
  std::vector<int> ranges;
  for (int i = start; i <= end; i *= 8) {
    ranges.push_back(i);
  }
  return ranges;
}

TEST_CASE("SpscQueue benchmarks", "[benchmark][internal][.]") {
  BENCHMARK("spsc_queue_enqueue_1_byte") {
    SpscQueue queue;
    queue.reserve(kDefaultQueueCapacity);
    uint8_t data = 0xAA;
    return queue.enqueue_bytes(&data);
  };

  for (int range_val : generate_range(8, kDefaultQueueCapacity / 2)) {
    BENCHMARK("spsc_queue_dequeue_1_byte/range:" + std::to_string(range_val)) {
      SpscQueue queue;
      queue.reserve(kDefaultQueueCapacity);
      uint8_t data = 0xAA;
      for (int i = 0; i < range_val; ++i) {
        queue.enqueue_bytes(&data);
      }
      return queue.dequeue_bytes(&data);
    };
  }

  BENCHMARK("spsc_queue_enqueue_16_bytes") {
    SpscQueue queue;
    queue.reserve(kDefaultQueueCapacity);
    uint8_t data[16] = {0};
    return queue.enqueue_bytes(data, sizeof(data));
  };

  for (int range_val : generate_range(32, kDefaultQueueCapacity / 2)) {
    BENCHMARK("spsc_queue_dequeue_16_bytes/range:" +
              std::to_string(range_val)) {
      SpscQueue queue;
      queue.reserve(kDefaultQueueCapacity);
      uint8_t data[16] = {0};
      for (size_t i = 0; i < static_cast<size_t>(range_val) / sizeof(data);
           ++i) {
        queue.enqueue_bytes(data, sizeof(data));
      }
      return queue.dequeue_bytes(data, sizeof(data));
    };
  }

  BENCHMARK("spsc_queue_enqueue_64_bytes") {
    SpscQueue queue;
    queue.reserve(kDefaultQueueCapacity);
    uint8_t data[64] = {0};
    return queue.enqueue_bytes(data, sizeof(data));
  };

  for (int range_val : generate_range(128, kDefaultQueueCapacity / 2)) {
    BENCHMARK("spsc_queue_dequeue_64_bytes/range:" +
              std::to_string(range_val)) {
      SpscQueue queue;
      queue.reserve(kDefaultQueueCapacity);
      uint8_t data[64] = {0};
      for (size_t i = 0; i < static_cast<size_t>(range_val) / sizeof(data);
           ++i) {
        queue.enqueue_bytes(data, sizeof(data));
      }
      return queue.dequeue_bytes(data, sizeof(data));
    };
  }

  BENCHMARK("spsc_queue_enqueue_peek_dequeue_16_bytes") {
    SpscQueue queue;
    queue.reserve(kDefaultQueueCapacity);
    uint8_t data_in[16] = {0};
    uint8_t data_out[16] = {0};
    queue.enqueue_bytes(data_in, sizeof(data_in));
    queue.peek_bytes(data_out, sizeof(data_out));
    return queue.dequeue_bytes(data_out, sizeof(data_out));
  };

  BENCHMARK("spsc_queue_enqueue_1_byte_medium_capacity") {
    SpscQueue queue;
    queue.reserve(kMediumQueueCapacity);
    uint8_t data = 0xAA;
    return queue.enqueue_bytes(&data);
  };

  for (int range_val : generate_range(8, kMediumQueueCapacity / 2)) {
    BENCHMARK("spsc_queue_dequeue_1_byte_medium_capacity/range:" +
              std::to_string(range_val)) {
      SpscQueue queue;
      queue.reserve(kMediumQueueCapacity);
      uint8_t data = 0xAA;
      for (int i = 0; i < range_val; ++i) {
        queue.enqueue_bytes(&data);
      }
      return queue.dequeue_bytes(&data);
    };
  }

  BENCHMARK("spsc_queue_enqueue_64_bytes_medium_capacity") {
    SpscQueue queue;
    queue.reserve(kMediumQueueCapacity);
    uint8_t data[64] = {0};
    return queue.enqueue_bytes(data, sizeof(data));
  };

  for (int range_val : generate_range(128, kMediumQueueCapacity / 2)) {
    BENCHMARK("spsc_queue_dequeue_64_bytes_medium_capacity/range:" +
              std::to_string(range_val)) {
      SpscQueue queue;
      queue.reserve(kMediumQueueCapacity);
      uint8_t data[64] = {0};
      for (size_t i = 0; i < static_cast<size_t>(range_val) / sizeof(data);
           ++i) {
        queue.enqueue_bytes(data, sizeof(data));
      }
      return queue.dequeue_bytes(data, sizeof(data));
    };
  }

  BENCHMARK("spsc_queue_enqueue_1_byte_large_capacity") {
    SpscQueue queue;
    queue.reserve(kLargeQueueCapacity);
    uint8_t data = 0xAA;
    return queue.enqueue_bytes(&data);
  };

  for (int range_val : generate_range(8, kLargeQueueCapacity / 2)) {
    BENCHMARK("spsc_queue_dequeue_1_byte_large_capacity/range:" +
              std::to_string(range_val)) {
      SpscQueue queue;
      queue.reserve(kLargeQueueCapacity);
      uint8_t data = 0xAA;
      for (int i = 0; i < range_val; ++i) {
        queue.enqueue_bytes(&data);
      }
      return queue.dequeue_bytes(&data);
    };
  }

  BENCHMARK("spsc_queue_enqueue_64_bytes_large_capacity") {
    SpscQueue queue;
    queue.reserve(kLargeQueueCapacity);
    uint8_t data[64] = {0};
    return queue.enqueue_bytes(data, sizeof(data));
  };

  for (int range_val : generate_range(128, kLargeQueueCapacity / 2)) {
    BENCHMARK("spsc_queue_dequeue_64_bytes_large_capacity/range:" +
              std::to_string(range_val)) {
      SpscQueue queue;
      queue.reserve(kLargeQueueCapacity);
      uint8_t data[64] = {0};
      for (size_t i = 0; i < static_cast<size_t>(range_val) / sizeof(data);
           ++i) {
        queue.enqueue_bytes(data, sizeof(data));
      }
      return queue.dequeue_bytes(data, sizeof(data));
    };
  }

  BENCHMARK("spsc_queue_medium_consumer_bench_with_busy_producer") {
    SpscQueue queue;
    queue.reserve(kMediumQueueCapacity);
    std::atomic<bool> running{true};
    uint8_t data = 0xAA;

    std::thread producer([&] {
      while (running.load(std::memory_order_relaxed)) {
        queue.enqueue_bytes(&data);
        std::this_thread::yield();
      }
    });

    auto result = queue.dequeue_bytes(&data);

    running = false;
    producer.join();
    return result;
  };

  BENCHMARK("spsc_queue_medium_producer_bench_with_busy_consumer") {
    SpscQueue queue;
    queue.reserve(kMediumQueueCapacity);
    std::atomic<bool> running{true};
    uint8_t data = 0xAA;

    std::thread consumer([&] {
      while (running.load(std::memory_order_relaxed)) {
        queue.dequeue_bytes(&data);
        std::this_thread::yield();
      }
    });

    auto result = queue.enqueue_bytes(&data);

    running = false;
    consumer.join();
    return result;
  };

  BENCHMARK("spsc_queue_large_consumer_bench_with_busy_producer") {
    SpscQueue queue;
    queue.reserve(kLargeQueueCapacity);
    std::atomic<bool> running{true};
    uint8_t data = 0xAA;

    std::thread producer([&] {
      while (running.load(std::memory_order_relaxed)) {
        queue.enqueue_bytes(&data);
        std::this_thread::yield();
      }
    });

    auto result = queue.dequeue_bytes(&data);

    running = false;
    producer.join();
    return result;
  };

  BENCHMARK("spsc_queue_large_producer_bench_with_busy_consumer") {
    SpscQueue queue;
    queue.reserve(kLargeQueueCapacity);
    std::atomic<bool> running{true};
    uint8_t data = 0xAA;

    std::thread consumer([&] {
      while (running.load(std::memory_order_relaxed)) {
        queue.dequeue_bytes(&data);
        std::this_thread::yield();
      }
    });

    auto result = queue.enqueue_bytes(&data);

    running = false;
    consumer.join();
    return result;
  };
}

}  // namespace

}  // namespace femtolog::logging
