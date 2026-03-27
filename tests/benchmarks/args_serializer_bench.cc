// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <string>
#include <string_view>

#include "catch2/benchmark/catch_benchmark.hpp"
#include "catch2/catch_test_macros.hpp"
#include "logging/impl/args_serializer.h"

namespace femtolog::logging {

namespace {

TEST_CASE("ArgsSerializer benchmarks", "[benchmark][internal][.]") {
  BENCHMARK("args_serializer_serialize_ints") {
    ArgsSerializer serializer;
    // Returning the result to prevent optimization in Catch2 v3
    return serializer.serialize<"", false>(1, 2, 3, 4, 5);
  };

  BENCHMARK("args_serializer_serialize_strings") {
    ArgsSerializer serializer;
    const char* a = "hello";
    std::string_view b = "world!";
    return serializer.serialize<"", false>(a, b);
  };

  BENCHMARK("args_serializer_serialize_mixed") {
    ArgsSerializer serializer;
    std::string_view s = "mixed";
    return serializer.serialize<"", false>(42, true, s, 3.14);
  };
}

}  // namespace

}  // namespace femtolog::logging
