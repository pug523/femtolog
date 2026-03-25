// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <chrono>
#include <string>
#include <thread>

#include "benchmarks/benchmark_util.h"
#include "catch2/benchmark/catch_benchmark.hpp"
#include "catch2/catch_test_macros.hpp"
#include "femtolog/logger.h"
#include "femtolog/options.h"
#include "sinks/file_sink.h"
#include "sinks/null_sink.h"
#include "sinks/stdout_sink.h"

namespace femtolog {

namespace {

Logger& setup_logger() {
  static bool initialized = false;

  Logger& logger = Logger::logger();
  if (!initialized) {
    logger.init(kFastOptions);
    logger.register_sink<NullSink>();
    // logger.register_sink<FileSink>(
    //     bench::get_benchmark_log_path("femtolog.log"));

    initialized = true;
  }
  logger.start_worker();
  std::this_thread::sleep_for(std::chrono::milliseconds(25));

  return logger;
}

inline void cleanup_logger(Logger* logger) {
  logger->stop_worker();
  logger->register_sink<StdoutSink<>>();
  logger->start_worker();
  logger->info<"Dropped count: {}\n">(logger->dropped_count());
  logger->flush();
  logger->stop_worker();
  logger->reset_count();
}

TEST_CASE("femtolog logging benchmarks", "[benchmark][logger][femtolog][.]") {
  Logger& logger = setup_logger();

  BENCHMARK("femtolog_info_literal") {
    logger.info<"Benchmark test message\n">();
    return 0;
  };

  BENCHMARK("femtolog_info_format_int") {
    logger.info<"Value: {}\n">(123);
    return 0;
  };

  BENCHMARK("femtolog_info_format_multi_int") {
    logger.info<"A: {}, B: {}, C: {}\n">(1, 2, 3);
    return 0;
  };

  BENCHMARK("femtolog_info_format_small_string") {
    std::string name = "benchmark_user";
    logger.info<"User: {}\n">(name);
    return 0;
  };

  BENCHMARK("femtolog_info_format_small_string_view") {
    std::string_view sv = "benchmark_view";
    logger.info<"View: {}\n">(sv);
    return 0;
  };

  BENCHMARK("femtolog_info_format_mixed") {
    std::string user = "user42";
    std::string_view op = "login";
    bool success = true;
    int64_t id = 9876543210;
    logger.info<"User: {}, Op: {}, Success: {}, ID: {}\n">(user, op, success,
                                                           id);
    return 0;
  };

  BENCHMARK("femtolog_info_format_large_string") {
    std::string long_str = std::string(512, 'X');
    logger.info<"Payload: {}\n">(long_str);
    return 0;
  };

  BENCHMARK("femtolog_info_ref_literal") {
    logger.info_ref<"Benchmark test message\n">();
    return 0;
  };

  BENCHMARK("femtolog_info_ref_format_int") {
    logger.info_ref<"Value: {}\n">(123);
    return 0;
  };

  BENCHMARK("femtolog_info_ref_format_multi_int") {
    logger.info_ref<"A: {}, B: {}, C: {}\n">(1, 2, 3);
    return 0;
  };

  BENCHMARK("femtolog_info_ref_format_small_string") {
    std::string name = "benchmark_user";
    logger.info_ref<"User: {}\n">(name);
    return 0;
  };

  BENCHMARK("femtolog_info_ref_format_small_string_view") {
    std::string_view sv = "benchmark_view";
    logger.info_ref<"View: {}\n">(sv);
    return 0;
  };

  BENCHMARK("femtolog_info_ref_format_mixed") {
    std::string user = "user42";
    std::string_view op = "login";
    bool success = true;
    int64_t id = 9876543210;
    logger.info_ref<"User: {}, Op: {}, Success: {}, ID: {}\n">(user, op,
                                                               success, id);
    return 0;
  };

  BENCHMARK("femtolog_info_ref_format_large_string") {
    std::string long_str = std::string(512, 'X');
    logger.info_ref<"Payload: {}\n">(long_str);
    return 0;
  };

  cleanup_logger(&logger);
}

}  // namespace

}  // namespace femtolog
