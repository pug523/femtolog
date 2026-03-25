// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <array>
#include <format>
#include <string>
#include <string_view>

#include "catch2/benchmark/catch_benchmark.hpp"
#include "catch2/catch_test_macros.hpp"
#include "fmt/core.h"
#include "fmt/format.h"

namespace {

constexpr const int value = 42;
constexpr const std::string_view name = "Foo";
constexpr const double pi = 3.14159;

TEST_CASE("format benchmarks", "[benchmark][fmt][.]") {
  BENCHMARK("format_std_format_simple") {
    return std::format("value = {}", value);
  };

  BENCHMARK("format_fmt_format_simple") {
    return fmt::format("value = {}", value);
  };

  BENCHMARK("format_std_format") {
    return std::format("value = {}, name = {}, pi = {:.2f}", value, name, pi);
  };

  BENCHMARK("format_fmt_format") {
    return fmt::format("value = {}, name = {}, pi = {:.2f}", value, name, pi);
  };

  BENCHMARK("format_std_format_to_n_simple") {
    std::array<char, 128> buffer;
    return std::format_to_n(buffer.data(), buffer.size(), "value = {}", value);
  };

  BENCHMARK("format_fmt_format_to_n_simple") {
    std::array<char, 128> buffer;
    return fmt::format_to_n(buffer.data(), buffer.size(), "value = {}", value);
  };

  BENCHMARK("format_fmt_format_to_n_simple_wo_fmt_string") {
    std::array<char, 128> buffer;
    return fmt::format_to_n(buffer.data(), buffer.size(), "value = {}", value);
  };

  BENCHMARK("format_std_format_to_n") {
    std::array<char, 128> buffer;
    return std::format_to_n(buffer.data(), buffer.size(),
                            "value = {}, name = {}, pi = {:.2f}", value, name,
                            pi);
  };

  BENCHMARK("format_fmt_format_to_n") {
    std::array<char, 128> buffer;
    return fmt::format_to_n(buffer.data(), buffer.size(),
                            "value = {}, name = {}, pi = {:.2f}", value, name,
                            pi);
  };

  BENCHMARK("format_fmt_format_dynamic") {
    const char* format_str = "value = {}, name = {}, pi = {:.2f}";
    std::string _ = fmt::format(fmt::runtime(format_str), value, name, pi);
    return 0;
  };
}

}  // namespace
