// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

#include "femtolog/base/format_util.h"
#include "femtolog/logging/impl/backend_worker.h"
#include "femtolog/logging/impl/internal_logger.h"
#include "femtolog/options.h"
#include "femtolog/sinks/sink_base.h"

namespace femtolog {

class Logger {
  using InternalLogger = logging::InternalLogger;
  using BackendWorker = logging::BackendWorker;

 public:
  ~Logger() = default;

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  Logger(Logger&&) noexcept = default;
  Logger& operator=(Logger&&) noexcept = default;

  inline void init(const FemtologOptions& options = FemtologOptions()) {
    internal_logger_->init(options);
  }

  template <
      typename T,
      typename = typename std::enable_if_t<std::is_base_of_v<SinkBase, T>>,
      typename... Args>
  inline void register_sink(Args&&... args) {
    internal_logger_->register_sink(
        std::make_unique<T>(std::forward<Args>(args)...));
  }

  inline void clear_sinks() { internal_logger_->clear_sinks(); }

  inline void start_worker() { internal_logger_->start_worker(); }

  inline void stop_worker() { internal_logger_->stop_worker(); }

  template <base::LogLevel level, base::FixedString fmt, typename... Args>
  inline constexpr void log(Args&&... args) noexcept {
    internal_logger_->log<level, fmt, false, Args...>(
        std::forward<Args>(args)...);
  }

  template <base::FixedString fmt, typename... Args>
  inline constexpr void raw(Args&&... args) noexcept {
    log<base::LogLevel::kRaw, fmt>(std::forward<Args>(args)...);
  }
  template <base::FixedString fmt, typename... Args>
  inline constexpr void fatal(Args&&... args) noexcept {
    log<base::LogLevel::kFatal, fmt>(std::forward<Args>(args)...);
  }
  template <base::FixedString fmt, typename... Args>
  inline constexpr void error(Args&&... args) noexcept {
    log<base::LogLevel::kError, fmt>(std::forward<Args>(args)...);
  }
  template <base::FixedString fmt, typename... Args>
  inline constexpr void warn(Args&&... args) noexcept {
    log<base::LogLevel::kWarn, fmt>(std::forward<Args>(args)...);
  }
  template <base::FixedString fmt, typename... Args>
  inline constexpr void info(Args&&... args) noexcept {
    log<base::LogLevel::kInfo, fmt>(std::forward<Args>(args)...);
  }
  template <base::FixedString fmt, typename... Args>
  inline constexpr void debug(Args&&... args) noexcept {
    log<base::LogLevel::kDebug, fmt>(std::forward<Args>(args)...);
  }
  template <base::FixedString fmt, typename... Args>
  inline constexpr void trace(Args&&... args) noexcept {
    log<base::LogLevel::kTrace, fmt>(std::forward<Args>(args)...);
  }

  template <base::LogLevel level, base::FixedString fmt, typename... Args>
  inline constexpr void log_ref(Args&&... args) noexcept {
    static_assert(((std::is_lvalue_reference_v<Args> ||
                    std::is_trivially_copyable_v<Args>) &&
                   ...),
                  "Args must all be l-value references.");
    internal_logger_->log<level, fmt, true, Args...>(
        std::forward<Args>(args)...);
  }

  template <base::FixedString fmt, typename... Args>
  inline constexpr void raw_ref(Args&&... args) noexcept {
    static_assert(sizeof...(Args) > 0, "use `raw` instead of `raw_ref`");
    log_ref<base::LogLevel::kRaw, fmt>(std::forward<Args>(args)...);
  }
  template <base::FixedString fmt, typename... Args>
  inline constexpr void fatal_ref(Args&&... args) noexcept {
    log_ref<base::LogLevel::kFatal, fmt>(std::forward<Args>(args)...);
  }
  template <base::FixedString fmt, typename... Args>
  inline constexpr void error_ref(Args&&... args) noexcept {
    log_ref<base::LogLevel::kError, fmt>(std::forward<Args>(args)...);
  }
  template <base::FixedString fmt, typename... Args>
  inline constexpr void warn_ref(Args&&... args) noexcept {
    log_ref<base::LogLevel::kWarn, fmt>(std::forward<Args>(args)...);
  }
  template <base::FixedString fmt, typename... Args>
  inline constexpr void info_ref(Args&&... args) noexcept {
    log_ref<base::LogLevel::kInfo, fmt>(std::forward<Args>(args)...);
  }
  template <base::FixedString fmt, typename... Args>
  inline constexpr void debug_ref(Args&&... args) noexcept {
    log_ref<base::LogLevel::kDebug, fmt>(std::forward<Args>(args)...);
  }
  template <base::FixedString fmt, typename... Args>
  inline constexpr void trace_ref(Args&&... args) noexcept {
    log_ref<base::LogLevel::kTrace, fmt>(std::forward<Args>(args)...);
  }

  inline void flush() noexcept { internal_logger_->flush(); }

  inline void level(base::LogLevel level) { internal_logger_->level(level); }

  inline void level(const char* level_str) {
    base::LogLevel new_level = base::log_level_from_string(level_str);
    if (new_level != base::LogLevel::kUnknown) {
      level(new_level);
    }
  }

  inline base::LogLevel level() const { return internal_logger_->level(); }

  inline size_t enqueued_count() const {
    return internal_logger_->enqueued_count();
  }

  inline size_t dropped_count() const {
    return internal_logger_->dropped_count();
  }

  inline void reset_count() { internal_logger_->reset_count(); }

  static Logger& logger();

  static Logger& global_logger();

  inline static Logger create_logger() { return Logger(); }

 private:
  Logger() : internal_logger_(std::make_unique<InternalLogger>()) {}

  std::unique_ptr<InternalLogger> internal_logger_ = nullptr;
};

}  // namespace femtolog
