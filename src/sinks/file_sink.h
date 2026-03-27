// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <fcntl.h>

#include <cstring>
#include <memory>
#include <string>

#include "base/file_util.h"
#include "base/log_entry.h"
#include "base/log_level.h"
#include "build/build_config.h"
#include "fmt/chrono.h"
#include "fmt/format.h"
#include "sinks/sink_base.h"

#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
#include <io.h>
#include <windows.h>
#else
#include <sys/uio.h>
#include <unistd.h>
#endif

namespace femtolog {

class FileSink final : public SinkBase {
 public:
  explicit FileSink(const std::string& file_path) : file_path_(file_path) {
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

    buffer_ = std::make_unique<char[]>(kBufferCapacity);
  }

  FileSink()
      : FileSink(base::join_path(base::exe_dir(), "logs", "latest.log")) {}

  ~FileSink() override {
    flush();

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
    constexpr size_t kTimestampBufSize = 32;
    char timestamp_buf[kTimestampBufSize];
    const size_t timestamp_size =
        format_timestamp<TimeZone::kLocal, "[{:%H:%M:%S}.{:09d}] ">(
            entry.timestamp_ns, timestamp_buf, kTimestampBufSize);

    const char* level_str = log_level_to_lower_str(entry.level);
    const size_t lv_len = level_len(entry.level);

    const size_t total = timestamp_size + lv_len + kSepLen + len;

    if (cursor_ + total > kBufferCapacity) {
      flush();
    }

    if (total > kBufferCapacity) {
      write_direct(timestamp_buf, timestamp_size, level_str, lv_len, content,
                   len);
      return;
    }

    char* buffer = buffer_.get();
    std::memcpy(buffer + cursor_, timestamp_buf, timestamp_size);
    cursor_ += timestamp_size;
    std::memcpy(buffer + cursor_, level_str, lv_len);
    cursor_ += lv_len;
    std::memcpy(buffer + cursor_, kSep, kSepLen);
    cursor_ += kSepLen;
    std::memcpy(buffer + cursor_, content, len);
    cursor_ += len;
  }

 private:
  inline void flush() {
    if (cursor_ == 0 || !buffer_ || fd_ < 0) {
      return;
    }
#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
    _write(fd_, buffer_.get(), static_cast<unsigned int>(cursor_));
#else
    [[maybe_unused]] const auto _ = write(fd_, buffer_.get(), cursor_);
#endif
    cursor_ = 0;
  }

  inline void write_direct(const char* timestamp_buf,
                           size_t timestamp_size,
                           const char* level_str,
                           size_t level_len,
                           const char* content,
                           size_t content_len) {
    if (fd_ < 0) {
      return;
    }

#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
    _write(fd_, timestamp_buf, static_cast<unsigned int>(timestamp_size));
    _write(fd_, level_str, static_cast<unsigned int>(level_len));
    _write(fd_, kSep, static_cast<unsigned int>(kSepLen));
    _write(fd_, content, static_cast<unsigned int>(content_len));
#else
    constexpr int kIovCount = 4;
    struct iovec iov[kIovCount];
    iov[0] = {const_cast<char*>(timestamp_buf), timestamp_size};
    iov[1] = {const_cast<char*>(level_str), level_len};
    iov[2] = {const_cast<char*>(kSep), kSepLen};
    iov[3] = {const_cast<char*>(content), content_len};
    [[maybe_unused]] const auto _ = writev(fd_, iov, kIovCount);
#endif
  }

  int fd_ = -1;

  std::string file_path_;
  std::unique_ptr<char[]> buffer_;
  size_t cursor_ = 0;

  static constexpr size_t kBufferCapacity = 4096;

  static constexpr const char* kSep = ": ";
  static constexpr size_t kSepLen = 2;
};

}  // namespace femtolog
