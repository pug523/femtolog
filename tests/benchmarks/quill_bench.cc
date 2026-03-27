// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <thread>

#include "catch2/benchmark/catch_benchmark.hpp"
#include "catch2/catch_test_macros.hpp"
#include "options.h"  // NOLINT(build/include_subdir)

#define QUILL_NO_EXCEPTIONS
#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/FileSink.h"
#include "quill/sinks/NullSink.h"

namespace quill {

namespace {

struct CustomizedFrontendOptions {
  static constexpr QueueType queue_type = QueueType::BoundedDropping;
  static constexpr size_t initial_queue_capacity =
      femtolog::kFastOptions.spsc_queue_size;
  static constexpr size_t unbounded_queue_max_capacity =
      femtolog::kFastOptions.spsc_queue_size;
  static constexpr HugePagesPolicy huge_pages_policy = HugePagesPolicy::Never;
};

using OptimizedFrontend = FrontendImpl<CustomizedFrontendOptions>;
using QuillLogger = LoggerImpl<CustomizedFrontendOptions>;

QuillLogger* setup_logger() {
  BackendOptions backend_options;
  backend_options.cpu_affinity = 5;
  backend_options.sleep_duration = std::chrono::nanoseconds{0};
  Backend::start(backend_options);
  std::this_thread::sleep_for(std::chrono::milliseconds(25));

  QuillLogger* logger = OptimizedFrontend::create_or_get_logger(
      "root", OptimizedFrontend::create_or_get_sink<NullSink>("null"));
  return logger;
}

void cleanup_logger() {
  Backend::stop();
}

TEST_CASE("quill logging benchmarks", "[benchmark][logger][quill][.]") {
  QuillLogger* logger = setup_logger();

  BENCHMARK("quill_info_literal") {
    QUILL_LOG_INFO(logger, "Benchmark test message");
    return;
  };

  BENCHMARK("quill_info_format_int") {
    QUILL_LOG_INFO(logger, "Value: {}", 123);
    return;
  };

  BENCHMARK("quill_info_format_multi_int") {
    QUILL_LOG_INFO(logger, "A: {}, B: {}, C: {}", 1, 2, 3);
    return;
  };

  BENCHMARK("quill_info_format_small_string") {
    std::string user = "benchmark_user";
    QUILL_LOG_INFO(logger, "User: {}", user);
    return;
  };

  BENCHMARK("quill_info_format_small_string_view") {
    std::string_view sv = "benchmark_view";
    QUILL_LOG_INFO(logger, "View: {}", sv);
    return;
  };

  BENCHMARK("quill_info_format_mixed") {
    std::string user = "user42";
    std::string_view op = "login";
    bool success = true;
    int64_t id = 9876543210;
    QUILL_LOG_INFO(logger, "User: {}, Op: {}, Success: {}, ID: {}", user, op,
                   success, id);
    return;
  };

  BENCHMARK("quill_info_format_large_string") {
    std::string payload(512, 'X');
    QUILL_LOG_INFO(logger, "Payload: {}", payload);
    return;
  };

  cleanup_logger();
}

}  // namespace

}  // namespace quill
