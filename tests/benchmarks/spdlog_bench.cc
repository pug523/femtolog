// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "benchmarks/benchmark_util.h"
#include "catch2/benchmark/catch_benchmark.hpp"
#include "catch2/catch_test_macros.hpp"

#define SPDLOG_NO_EXCEPTIONS
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/spdlog.h"

namespace spdlog {

namespace {

std::shared_ptr<logger> setup_logger() {
  auto null_sink = std::make_shared<sinks::null_sink_mt>();
  auto l = std::make_shared<logger>("spd", null_sink);
  set_default_logger(l);
  set_level(level::info);

  std::this_thread::sleep_for(std::chrono::milliseconds(25));
  return l;
}

TEST_CASE("spdlog logging benchmarks", "[benchmark][logger][spdlog][.]") {
  auto l = setup_logger();

  BENCHMARK("spdlog_info_literal") {
    SPDLOG_INFO("Benchmark test message");
    return;
  };

  BENCHMARK("spdlog_info_format_int") {
    SPDLOG_INFO("Value: {}", 123);
    return;
  };

  BENCHMARK("spdlog_info_format_multi_int") {
    SPDLOG_INFO("A: {}, B: {}, C: {}", 1, 2, 3);
    return;
  };

  BENCHMARK("spdlog_info_format_small_string") {
    std::string user = "benchmark_user";
    SPDLOG_INFO("User: {}", user);
    return;
  };

  BENCHMARK("spdlog_info_format_small_string_view") {
    std::string_view sv = "benchmark_view";
    SPDLOG_INFO("View: {}", sv);
    return;
  };

  BENCHMARK("spdlog_info_format_mixed") {
    std::string user = "user42";
    std::string_view op = "login";
    bool success = true;
    int64_t id = 9876543210;
    SPDLOG_INFO("User: {}, Op: {}, Success: {}, ID: {}", user, op, success, id);
    return;
  };

  BENCHMARK("spdlog_info_format_large_string") {
    std::string payload(512, 'X');
    SPDLOG_INFO("Payload: {}", payload);
    return;
  };
}

}  // namespace

}  // namespace spdlog
