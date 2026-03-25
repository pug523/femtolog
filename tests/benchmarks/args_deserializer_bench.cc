// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <string>

#include "catch2/benchmark/catch_benchmark.hpp"
#include "catch2/catch_test_macros.hpp"
#include "logging/impl/args_deserializer.h"
#include "logging/impl/args_serializer.h"

namespace femtolog::logging {

namespace {

TEST_CASE("args_deserializer_deserialize_and_format benchmark",
          "[benchmark][internal][.]") {
  int x = 100;
  std::string y = "benchmark";
  double z = 9.99;

  ArgsSerializer<256> serializer;
  auto& args = serializer.serialize<"x={}, y={}, z={}", false>(x, y, z);

  const base::SerializedArgsHeader* header =
      reinterpret_cast<const base::SerializedArgsHeader*>(args.data());
  const char* payload =
      reinterpret_cast<const char*>(args.data() + sizeof(*header));

  fmt::memory_buffer buf;
  buf.reserve(1024);

  BENCHMARK("deserialize_and_format") {
    size_t n =
        header->deserialize_and_format_func(&buf, header->format_func, payload);
    return n;
  };
}

}  // namespace

}  // namespace femtolog::logging
