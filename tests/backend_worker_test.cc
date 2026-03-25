// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "logging/impl/backend_worker.h"

#include <limits>
#include <memory>
#include <utility>

#include "catch2/catch_test_macros.hpp"
#include "sinks/null_sink.h"

namespace femtolog::logging {

TEST_CASE("RegisterAndClearSinks", "[BackendWorkerTest]") {
  BackendWorker worker;
  auto sink = std::make_unique<NullSink>();
  FemtologOptions options;
  SpscQueue queue;
  worker.init(&queue, options);

  worker.register_sink(std::move(sink));
  worker.clear_sinks();
}

TEST_CASE("CPUAffinityNoCrash", "[BackendWorkerTest]") {
  BackendWorker worker;

  auto sink = std::make_unique<NullSink>();
  FemtologOptions options;
  options.backend_worker_cpu_affinity = 5;
  SpscQueue queue;
  worker.init(&queue, options);
  worker.register_sink(std::move(sink));
  worker.start();
  worker.stop();
}

}  // namespace femtolog::logging
