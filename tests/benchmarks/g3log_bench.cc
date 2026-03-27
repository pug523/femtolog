// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <thread>

#include "catch2/benchmark/catch_benchmark.hpp"
#include "catch2/catch_test_macros.hpp"
#include "g3log/g3log.hpp"
#include "g3log/logworker.hpp"

namespace g3 {

namespace {

struct NullSink {
  // NOLINTNEXTLINE(performance-unnecessary-value-param)
  void receive(g3::LogMessageMover /*log*/) {}
};

std::unique_ptr<g3::LogWorker> setup_logger() {
  auto worker = g3::LogWorker::createLogWorker();
  worker->addSink(std::make_unique<NullSink>(), &NullSink::receive);

  g3::initializeLogging(worker.get());

  std::this_thread::sleep_for(std::chrono::milliseconds(25));
  return worker;
}

TEST_CASE("g3log logging benchmarks", "[benchmark][logger][g3log][.]") {
  auto worker = setup_logger();

  BENCHMARK("g3log_info_literal") {
    LOG(INFO) << "Benchmark test message";
    return;
  };

  BENCHMARK("g3log_info_format_int") {
    LOG(INFO) << "Value: " << 123;
    return;
  };

  BENCHMARK("g3log_info_format_multi_int") {
    LOG(INFO) << "A: " << 1 << ", B: " << 2 << ", C: " << 3;
    return;
  };

  BENCHMARK("g3log_info_format_small_string") {
    std::string user = "benchmark_user";
    LOG(INFO) << "User: " << user;
    return;
  };

  BENCHMARK("g3log_info_format_small_string_view") {
    std::string_view sv = "benchmark_view";
    LOG(INFO) << "View: " << sv;
    return;
  };

  BENCHMARK("g3log_info_format_mixed") {
    std::string user = "user42";
    std::string_view op = "login";
    bool success = true;
    int64_t id = 9876543210;
    LOG(INFO) << "User: " << user << ", Op: " << op
              << ", Success: " << (success ? "true" : "false")
              << ", ID: " << id;
    return;
  };

  BENCHMARK("g3log_info_format_large_string") {
    std::string payload(512, 'X');
    LOG(INFO) << "Payload: " << payload;
    return;
  };
}

}  // namespace

}  // namespace g3
