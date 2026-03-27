// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstdint>
#include <ctime>

#include "base/check.h"
#include "base/format_util.h"
#include "base/log_entry.h"
#include "build/build_config.h"
#include "fmt/chrono.h"
#include "fmt/format.h"

#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace femtolog {

enum class TimeZone : uint8_t {
  kUtc = 0,
  kLocal = 1,
};

class SinkBase {
 public:
  explicit SinkBase() = default;
  virtual ~SinkBase() = default;

  SinkBase(const SinkBase&) = delete;
  SinkBase& operator=(const SinkBase&) = delete;

  SinkBase(SinkBase&&) noexcept = default;
  SinkBase& operator=(SinkBase&&) noexcept = default;

  virtual inline void on_log(const base::LogEntry& entry,
                             const char* content,
                             size_t len) = 0;

 protected:
  template <TimeZone tz = TimeZone::kLocal,
            base::FixedString fmt = "{:%H:%M:%S}.{:09d} ">
  static size_t format_timestamp(uint64_t time_ns, char* buf, size_t buf_size) {
    FEMTOLOG_DCHECK(buf);
    FEMTOLOG_DCHECK_GT(buf_size, fmt.size);

    // separate seconds and nanoseconds
    time_t seconds = static_cast<time_t>(time_ns / 1'000'000'000);
    uint32_t nanoseconds = static_cast<uint32_t>(time_ns % 1'000'000'000);

    // convert seconds to struct tm (thread-safe)
    std::tm tm_buf;
    if constexpr (tz == TimeZone::kUtc) {
#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
      gmtime_s(&tm_buf, &seconds);
#else
      gmtime_r(&seconds, &tm_buf);
#endif
    } else if constexpr (tz == TimeZone::kLocal) {
#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
      localtime_s(&tm_buf, &seconds);
#else
      localtime_r(&seconds, &tm_buf);
#endif
    }

    auto result =
        fmt::format_to_n(buf, buf_size, fmt.data, tm_buf, nanoseconds);

    if (result.size > buf_size - 1) {
      return result.size;
    }

    buf[result.size] = '\0';
    return result.size;
  }
};

}  // namespace femtolog
