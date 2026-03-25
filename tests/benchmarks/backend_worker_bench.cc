// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <limits>
#include <memory>
#include <utility>

#include "catch2/benchmark/catch_benchmark.hpp"
#include "catch2/catch_test_macros.hpp"
#include "femtolog/logging/impl/backend_worker.h"
#include "femtolog/sinks/null_sink.h"

namespace femtolog::logging {

namespace {

TEST_CASE("BackendWorker benchmarks", "[benchmark][internal][.]") {
  BackendWorker worker;
  auto sink = std::make_unique<NullSink>();
  FemtologOptions options;
  options.backend_dequeue_buffer_size = 4096;
  options.backend_format_buffer_size = 4096;
  options.backend_worker_cpu_affinity = std::numeric_limits<size_t>::max();

  SpscQueue queue;
  worker.init(&queue, options);
  worker.register_sink(std::move(sink));

  BENCHMARK("backend_worker_run_loop") {
    worker.start();
    worker.stop();
    return 0;
  };
}

}  // namespace

}  // namespace femtolog::logging
