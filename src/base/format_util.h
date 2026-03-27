// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "build/build_config.h"
#include "fmt/args.h"
#include "fmt/core.h"
#include "fmt/format.h"

#if FEMTOLOG_BUILD_FLAG(IS_OS_LINUX)
#include <ctime>
#else
#include <chrono>
#endif

namespace femtolog::base {

template <size_t N>
struct FixedString {
  char data[N + 1]{};
  size_t size = N;

  // NOLINTNEXTLINE(google-explicit-constructor)
  consteval FixedString(const char (&str)[N + 1]) {
    for (size_t i = 0; i < N; ++i) {
      data[i] = str[i];
    }
    data[N] = '\0';
  }

  consteval std::string_view view() const {
    return std::string_view{data, size};
  }

  consteval const char* c_str() const { return data; }
};

template <size_t N>
FixedString(const char (&)[N]) -> FixedString<N - 1>;

inline uint64_t timestamp_ns() noexcept {
#if FEMTOLOG_BUILD_FLAG(IS_OS_LINUX)
  timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return static_cast<uint64_t>(ts.tv_sec) * 1000000000ull +
         static_cast<size_t>(ts.tv_nsec);
#else
  return static_cast<uint64_t>(
      std::chrono::steady_clock::now().time_since_epoch().count());
#endif
}

using FormatFunction = size_t (*)(fmt::memory_buffer*, const fmt::format_args&);

template <FixedString fmt>
struct FormatDispatcher {
  static size_t format(fmt::memory_buffer* buf, const fmt::format_args& args) {
    auto result =
        fmt::vformat_to_n(buf->data(), buf->capacity(), fmt.view(), args);
    return result.size;
  }

  static constexpr FormatFunction function() { return &format; }
};

}  // namespace femtolog::base
