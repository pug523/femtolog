// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstring>
#include <memory>
#include <mutex>

#include "base/log_entry.h"
#include "base/log_level.h"
#include "base/style.h"
#include "build/build_flag.h"
#include "femtolog/options.h"
#include "logging/impl/internal_logger.h"
#include "sinks/sink_base.h"

#if FEMTOLOG_IS_WINDOWS
#include <io.h>
#else
#include <sys/uio.h>
#include <unistd.h>
#endif

namespace femtolog {

template <bool enable_buffering = false, bool sync_write = true>
class StdoutSink final : public SinkBase {
 public:
  explicit StdoutSink(ColorMode mode = ColorMode::kAuto) : mode_(mode) {
    if constexpr (enable_buffering) {
      buffer_ = std::make_unique<char[]>(kBufferCapacity);
    }
  }

  ~StdoutSink() override {
    if constexpr (enable_buffering) {
      flush();
    }
  }

  inline void on_log(const base::LogEntry& entry,
                     const char* content,
                     size_t len) override {
    if constexpr (!enable_buffering) {
      write_direct(entry.level, content, len);
      return;
    }
    const size_t total_len = estimate_length(entry.level, len);
    if (cursor_ + total_len > kBufferCapacity) {
      flush();
    }
    if (total_len > kBufferCapacity) {
      write_direct(entry.level, content, len);
      return;
    }
    cursor_ +=
        build_message(buffer_.get() + cursor_, entry.level, content, len);
  }

 private:
  inline size_t estimate_length(base::LogLevel level,
                                size_t content_len) const {
    size_t len = content_len + kSepLen;
    if (is_color_enabled()) {
      len += base::kAnsiStyleLen + kResetLen;
    }
    if (level != base::LogLevel::kRaw) {
      len += kLogLevelPrefixLen;
    }
    return len;
  }

  inline size_t build_message(char* out,
                              base::LogLevel level,
                              const char* content,
                              size_t content_len) const {
    char* p = out;
    if (level != base::LogLevel::kRaw) {
      if (is_color_enabled()) {
        const char* col = log_level_to_ansi_style(level);
        std::memcpy(p, col, base::kAnsiStyleLen);
        p += base::kAnsiStyleLen;
        const char* prefix = log_level_to_prefix(level);
        std::memcpy(p, prefix, kLogLevelPrefixLen);
        p += kLogLevelPrefixLen;
        std::memcpy(p, base::kReset, kResetLen);
        p += kResetLen;
      } else {
        const char* prefix = log_level_to_prefix(level);
        std::memcpy(p, prefix, kLogLevelPrefixLen);
        p += kLogLevelPrefixLen;
      }
      std::memcpy(p, kSep, kSepLen);
      p += kSepLen;
    }
    std::memcpy(p, content, content_len);
    p += content_len;
    return static_cast<size_t>(p - out);
  }

  inline void flush() {
    if constexpr (enable_buffering) {
      if (cursor_ == 0) {
        return;
      }
      lock();
#if FEMTOLOG_IS_WINDOWS
      _write(kStdOutFd, buffer_.get(), static_cast<unsigned int>(cursor_));
#else
      const auto _ = write(kStdOutFd, buffer_.get(), cursor_);
#endif
      unlock();
      cursor_ = 0;
    }
  }

  inline void write_direct(base::LogLevel level,
                           const char* content,
                           size_t len) {
    char tmp[kBufferCapacity];
    size_t total = build_message(tmp, level, content, len);
    lock();
#if FEMTOLOG_IS_WINDOWS
    _write(kStdOutFd, tmp, static_cast<unsigned int>(total));
#else
    const auto _ = write(kStdOutFd, tmp, total);
#endif
    unlock();
  }

  inline static void lock() {
    if constexpr (sync_write) {
      stdout_mutex().lock();
    }
  }
  inline static void unlock() {
    if constexpr (sync_write) {
      stdout_mutex().unlock();
    }
  }

  inline bool is_color_enabled() const {
    return (mode_ == ColorMode::kAuto &&
            logging::InternalLogger::is_ansi_sequence_available()) ||
           mode_ == ColorMode::kAlways;
  }

  static std::mutex& stdout_mutex() {
    static std::mutex m;
    return m;
  }

  inline static constexpr const char* log_level_to_prefix(
      base::LogLevel level) {
    switch (level) {
      case base::LogLevel::kFatal: return "fatal";
      case base::LogLevel::kError: return "error";
      case base::LogLevel::kWarn: return " warn";
      case base::LogLevel::kInfo: return " info";
      case base::LogLevel::kDebug: return "debug";
      case base::LogLevel::kTrace: return "trace";
      default: FEMTOLOG_DCHECK(false); return "";
    }
  }

  static constexpr size_t kLogLevelPrefixLen = 5;

  std::unique_ptr<char[]> buffer_;
  size_t cursor_ = 0;
  ColorMode mode_;

  static constexpr int kStdOutFd = 1;
  static constexpr size_t kBufferCapacity = 4096;
  static constexpr const char* kSep = ": ";
  static constexpr size_t kSepLen = 2;
  static constexpr size_t kResetLen = 4;
};

}  // namespace femtolog
