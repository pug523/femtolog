// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "logging/impl/spsc_queue.h"

#include <string>
#include <vector>

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

namespace femtolog::logging {

namespace {

TEST_CASE("SpscQueue Constructor and Capacity", "[SpscQueueTest]") {
  SpscQueue queue_small;
  queue_small.reserve(16);
  CHECK(queue_small.capacity() == 16);
  CHECK(queue_small.empty());
  CHECK(queue_small.size() == 0);
  CHECK(queue_small.available_space() == 16);

  SpscQueue queue_large;
  queue_large.reserve(1024);
  CHECK(queue_large.capacity() == 1024);
  CHECK(queue_large.empty());
  CHECK(queue_large.size() == 0);
  CHECK(queue_large.available_space() == 1024);

  SpscQueue queue_non_power_of_2;
  queue_non_power_of_2.reserve(20);
  CHECK(queue_non_power_of_2.capacity() == 32);
  CHECK(queue_non_power_of_2.empty());
  CHECK(queue_non_power_of_2.size() == 0);
  CHECK(queue_non_power_of_2.available_space() == 32);
}

TEST_CASE("SpscQueue Enqueue Dequeue Single Element", "[SpscQueueTest]") {
  SpscQueue queue;
  queue.reserve(16);

  int data_in = 42;
  SpscQueueStatus result = queue.enqueue_bytes(&data_in);
  CHECK(result == SpscQueueStatus::kOk);
  CHECK_FALSE(queue.empty());
  CHECK(queue.size() == sizeof(int));
  CHECK(queue.available_space() == 16 - sizeof(int));

  int data_out = 0;
  result = queue.dequeue_bytes(&data_out, sizeof(int));
  CHECK(result == SpscQueueStatus::kOk);
  CHECK(queue.empty());
  CHECK(queue.size() == 0);
  CHECK(queue.available_space() == 16);
  CHECK(data_out == data_in);

  char char_in = 'A';
  result = queue.enqueue_bytes(&char_in);
  CHECK(result == SpscQueueStatus::kOk);
  CHECK_FALSE(queue.empty());
  CHECK(queue.size() == sizeof(char));

  char char_out = ' ';
  result = queue.dequeue_bytes(&char_out, sizeof(char));
  CHECK(result == SpscQueueStatus::kOk);
  CHECK(queue.empty());
  CHECK(char_out == char_in);
}

TEST_CASE("SpscQueue Fill and Overflow", "[SpscQueueTest]") {
  SpscQueue queue;
  queue.reserve(16);

  for (int i = 0; i < 16; ++i) {
    char data = static_cast<char>(i);
    SpscQueueStatus result = queue.enqueue_bytes(&data);
    CHECK(result == SpscQueueStatus::kOk);
  }
  CHECK(queue.size() == 16);
  CHECK(queue.available_space() == 0);
  CHECK_FALSE(queue.empty());

  char data_overflow = 'Z';
  SpscQueueStatus result = queue.enqueue_bytes(&data_overflow);
  CHECK(result == SpscQueueStatus::kOverflow);
  CHECK(queue.size() == 16);
}

TEST_CASE("SpscQueue Dequeue from Empty", "[SpscQueueTest]") {
  SpscQueue queue;
  queue.reserve(16);

  int data_out = 0;
  SpscQueueStatus result = queue.dequeue_bytes(&data_out, sizeof(int));
  CHECK(result == SpscQueueStatus::kUnderflow);
  CHECK(queue.empty());
  CHECK(queue.size() == 0);
}

TEST_CASE("SpscQueue Enqueue Dequeue Mixed Sizes", "[SpscQueueTest]") {
  SpscQueue queue;
  queue.reserve(64);

  int int_val_in = 12345;
  CHECK(queue.enqueue_bytes(&int_val_in) == SpscQueueStatus::kOk);

  double double_val_in = 3.14159;
  CHECK(queue.enqueue_bytes(&double_val_in) == SpscQueueStatus::kOk);

  struct MyStruct {
    char a;
    int16_t b;
  };

  MyStruct struct_in = {'X', 100};
  CHECK(queue.enqueue_bytes(&struct_in) == SpscQueueStatus::kOk);

  CHECK(queue.size() == sizeof(int) + sizeof(double) + sizeof(MyStruct));
  CHECK_FALSE(queue.empty());

  int int_val_out = 0;
  CHECK(queue.dequeue_bytes(&int_val_out, sizeof(int)) == SpscQueueStatus::kOk);
  CHECK(int_val_out == int_val_in);

  double double_val_out = 0.0;
  CHECK(queue.dequeue_bytes(&double_val_out, sizeof(double)) ==
        SpscQueueStatus::kOk);
  CHECK(double_val_out == Catch::Approx(double_val_in));

  MyStruct struct_out;
  CHECK(queue.dequeue_bytes(&struct_out, sizeof(MyStruct)) ==
        SpscQueueStatus::kOk);
  CHECK(struct_out.a == struct_in.a);
  CHECK(struct_out.b == struct_in.b);

  CHECK(queue.empty());
  CHECK(queue.size() == 0);
}

TEST_CASE("SpscQueue Wraparound Behavior", "[SpscQueueTest]") {
  SpscQueue queue;
  queue.reserve(16);

  std::vector<char> data_in(10, 'A');
  CHECK(queue.enqueue_bytes(data_in.data(), data_in.size()) ==
        SpscQueueStatus::kOk);
  CHECK(queue.size() == 10);

  std::vector<char> data_out(5);
  CHECK(queue.dequeue_bytes(data_out.data(), data_out.size()) ==
        SpscQueueStatus::kOk);
  CHECK(queue.size() == 5);

  std::vector<char> data_in_2(8, 'B');
  CHECK(queue.enqueue_bytes(data_in_2.data(), data_in_2.size()) ==
        SpscQueueStatus::kOk);
  CHECK(queue.size() == 13);
  CHECK(queue.available_space() == 16 - 13);

  std::vector<char> data_out_remaining(13);
  CHECK(queue.dequeue_bytes(data_out_remaining.data(), 13) ==
        SpscQueueStatus::kOk);
  CHECK(queue.empty());
  CHECK(queue.size() == 0);
}

TEST_CASE("SpscQueue Enqueue Too Many Bytes", "[SpscQueueTest]") {
  SpscQueue queue;
  queue.reserve(16);
  CHECK(queue.capacity() == 16);
  CHECK(queue.empty());

  std::vector<char> large_data(20, 'X');
  SpscQueueStatus result =
      queue.enqueue_bytes(large_data.data(), large_data.size());
  CHECK(result == SpscQueueStatus::kOverflow);
  CHECK(queue.empty());
  CHECK(queue.size() == 0);
}

TEST_CASE("SpscQueue Dequeue Too Many Bytes", "[SpscQueueTest]") {
  SpscQueue queue;
  queue.reserve(16);

  std::vector<char> data_in(5, 'A');
  CHECK(queue.enqueue_bytes(data_in.data(), data_in.size()) ==
        SpscQueueStatus::kOk);
  CHECK(queue.size() == 5);

  std::vector<char> data_out(10);
  SpscQueueStatus result =
      queue.dequeue_bytes(data_out.data(), data_out.size());
  CHECK(result == SpscQueueStatus::kUnderflow);
  CHECK(queue.size() == 5);
}

TEST_CASE("SpscQueue Custom Struct Enqueue Dequeue", "[SpscQueueTest]") {
  SpscQueue queue;
  queue.reserve(64);

  struct TestData {
    int id;
    float value;
    char name[10];
  };

  TestData data_in = {101, 3.14f, "TestName"};
  SpscQueueStatus result = queue.enqueue_bytes(&data_in);
  CHECK(result == SpscQueueStatus::kOk);
  CHECK(queue.size() == sizeof(TestData));

  TestData data_out;
  result = queue.dequeue_bytes(&data_out, sizeof(TestData));
  CHECK(result == SpscQueueStatus::kOk);
  CHECK(data_out.id == data_in.id);
  CHECK(data_out.value == Catch::Approx(data_in.value));
  CHECK(std::string(data_out.name) == std::string(data_in.name));
  CHECK(queue.empty());
}

}  // namespace

}  // namespace femtolog::logging
