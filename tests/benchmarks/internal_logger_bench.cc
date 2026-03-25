// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "catch2/benchmark/catch_benchmark.hpp"
#include "catch2/catch_test_macros.hpp"
#include "logging/impl/internal_logger.h"
#include "sinks/null_sink.h"

namespace femtolog::logging {

namespace {

TEST_CASE("InternalLogger benchmarks", "[benchmark][internal][.]") {
  InternalLogger logger;
  logger.init();
  logger.register_sink(std::make_unique<NullSink>());
  logger.start_worker();

  BENCHMARK("internal_logger_literal_log") {
    return logger.log<base::LogLevel::kInfo, "Benchmark literal", false>();
  };

  BENCHMARK("internal_logger_formatted_log") {
    const char* str = "times";
    return logger.log<base::LogLevel::kInfo, "Benchmark {} {}", false>(42, str);
  };

  logger.stop_worker();
}

}  // namespace

}  // namespace femtolog::logging
