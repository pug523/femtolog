// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <fcntl.h>

#include <algorithm>
#include <memory>
#include <string>

#include "base/file_util.h"
#include "base/log_entry.h"
#include "base/log_level.h"
#include "build/build_config.h"
#include "sinks/sink_base.h"

#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
#include <io.h>
#include <windows.h>
#else
#include <sys/uio.h>
#include <unistd.h>
#endif

namespace femtolog {

template <bool enable_buffering = true>
class JsonLinesSink final : public SinkBase {
 public:
  explicit JsonLinesSink(const std::string& file_path) : file_path_(file_path) {
    std::string parent_dir = base::parent_dir(file_path_);
    if (!base::dir_exists(parent_dir.c_str())) {
      base::create_directories(parent_dir.c_str());
    }

#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
    fd_ =
        _open(file_path_.c_str(), _O_WRONLY | _O_CREAT | _O_APPEND | _O_BINARY,
              _S_IREAD | _S_IWRITE);
#else
    fd_ = open(file_path_.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
#endif

    if constexpr (enable_buffering) {
      buffer_ = std::make_unique<char[]>(kBufferCapacity);
    }
  }

  JsonLinesSink()
      : JsonLinesSink(
            base::join_path(base::exe_dir(), "logs", "jsonl", "latest.jsonl")) {
  }

  ~JsonLinesSink() override {
    if constexpr (enable_buffering) {
      flush();
    }

    if (fd_ >= 0) {
#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
      _close(fd_);
#else
      close(fd_);
#endif
    }
  }

  inline void on_log(const base::LogEntry& entry,
                     const char* content,
                     size_t len) override {
    // Pre-format JSON line
    std::string encoded = base::encode_escape(content, len);
    const char* level_str = log_level_to_lower_str(entry.level);

    char line[kMaxJsonLineSize];
    auto result = fmt::format_to_n(
        line, kMaxJsonLineSize,
        R"({{"timestamp": {}, "level": "{}", "message": "{}"}})"
        "\n",
        entry.timestamp_ns, level_str, fmt::string_view(encoded));

    const size_t line_len = std::min(result.size, kMaxJsonLineSize);

    if constexpr (!enable_buffering) {
      write_raw(line, line_len);
      return;
    }

    if (cursor_ + line_len > kBufferCapacity) {
      flush();
    }

    std::memcpy(buffer_.get() + cursor_, line, line_len);
    cursor_ += line_len;
  }

 private:
  inline void flush() {
    if constexpr (enable_buffering) {
      if (cursor_ == 0 || !buffer_ || fd_ < 0) {
        return;
      }
#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
      _write(fd_, buffer_.get(), static_cast<uint32_t>(cursor_));
#else
      const auto _ = write(fd_, buffer_.get(), cursor_);
#endif
      cursor_ = 0;
    }
  }

  inline void write_raw(const char* data, size_t size) {
    if (fd_ < 0) {
      return;
    }

#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
    _write(fd_, data, static_cast<uint32_t>(size));
#else
    const auto _ = write(fd_, data, size);
#endif
  }

  int fd_ = -1;

  std::string file_path_;
  std::unique_ptr<char[]> buffer_;
  size_t cursor_ = 0;

  static constexpr size_t kBufferCapacity = 8192;
  static constexpr size_t kMaxJsonLineSize = 2048;
};

}  // namespace femtolog
