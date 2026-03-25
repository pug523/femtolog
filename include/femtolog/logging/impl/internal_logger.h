// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <memory>
#include <new>
#include <utility>

#include "femtolog/base/log_entry.h"
#include "femtolog/base/log_level.h"
#include "femtolog/base/string_registry.h"
#include "femtolog/logging/impl/args_serializer.h"
#include "femtolog/logging/impl/backend_worker.h"
#include "femtolog/logging/impl/spsc_queue.h"
#include "femtolog/options.h"
#include "femtolog/sinks/sink_base.h"

namespace femtolog::logging {

// 1KiB max per entry. (consider sizeof LogEntry)
// Use reference mode if you need to output strings longer than this limit
static constexpr const size_t kMaxPayloadSize = 1024 - sizeof(base::LogEntry);

class InternalLogger {
 public:
  InternalLogger();

  ~InternalLogger();

  InternalLogger(const InternalLogger&) = delete;
  InternalLogger& operator=(const InternalLogger&) = delete;

  InternalLogger(InternalLogger&&) noexcept = delete;
  InternalLogger& operator=(InternalLogger&&) noexcept = delete;

  void init(const FemtologOptions& options = FemtologOptions());

  void register_sink(std::unique_ptr<SinkBase> sink);

  void clear_sinks();

  void start_worker();

  void stop_worker();

  [[nodiscard]] SpscQueue& queue() noexcept { return queue_; }
  [[nodiscard]] const SpscQueue& queue() const noexcept { return queue_; }

  [[nodiscard]] inline uint32_t thread_id() const noexcept {
    return thread_id_;
  }

  [[nodiscard]] inline size_t enqueued_count() const noexcept {
    return enqueued_count_;
  }

  [[nodiscard]] inline size_t dropped_count() const noexcept {
    return dropped_count_;
  }

  inline void reset_count() noexcept {
    enqueued_count_ = 0;
    dropped_count_ = 0;
  }

  template <base::LogLevel level,
            base::FixedString fmt,
            bool ref_mode,
            typename... Args>
  inline void log(Args&&... args) noexcept {
    // Compile-time level check
    // Assuming `debug` is common threshold
    if constexpr (level > base::LogLevel::kDebug) {
      if (level > level_) [[unlikely]] {
        return;
      }
    } else {
      if (level > level_) [[likely]] {
        return;
      }
    }

    if (backend_worker_.status() != BackendWorkerStatus::kRunning)
        [[unlikely]] {
      return;
    }

    if constexpr (sizeof...(Args) == 0) {
      constexpr std::string_view view(fmt.data, fmt.size);
      log_literal<level>(view);
    } else {
      constexpr uint16_t format_id = base::StringRegistry::get_string_id<fmt>();
      string_registry_.register_string<fmt>();

      const auto& serialized_args =
          serializer_.serialize<fmt, ref_mode>(std::forward<Args>(args)...);
      log_serialized<level>(format_id, serialized_args);
    }
  }

  inline void flush() noexcept { backend_worker_.flush(); }

  inline void level(base::LogLevel level) noexcept { level_ = level; }

  [[nodiscard]] inline base::LogLevel level() const noexcept { return level_; }

  [[nodiscard]] static bool is_ansi_sequence_available();

 private:
  template <base::LogLevel level>
  inline constexpr void log_literal(const std::string_view& message) {
    const size_t payload_len = message.size();
    if (payload_len >= kMaxPayloadSize) [[unlikely]] {
      dropped_count_++;
      return;
    }

    const base::LogEntry* entry = base::LogEntry::create(
        entry_buffer_, thread_id_, base::kLiteralLogStringId, level, 0,
        message.data(), message.length());

    enqueue_log_entry(entry);
  }

  template <base::LogLevel level, size_t Capacity>
  inline constexpr void log_serialized(
      uint16_t format_id,
      const base::SerializedArgs<Capacity>& serialized) {
    // NOLINTNEXTLINE
    if (serialized.size() >= kMaxPayloadSize || serialized.size() == 0)
        [[unlikely]] {
      dropped_count_++;
      return;
    }

    const base::LogEntry* entry =
        base::LogEntry::create(entry_buffer_, thread_id_, format_id, level, 0,
                               serialized.data(), serialized.size());

    enqueue_log_entry(entry);
  }

  inline void enqueue_log_entry(const base::LogEntry* entry) noexcept;

  [[nodiscard]] static uint32_t current_thread_id() noexcept;

  friend void internal_logger_enqueue_log_entry_bench(InternalLogger* logger);

  // Hot data - frequently accessed (first cache line)
  alignas(base::kCacheSize) base::LogLevel level_ = base::LogLevel::kInfo;
  const uint32_t thread_id_;
  size_t enqueued_count_ = 0;
  size_t dropped_count_ = 0;
  base::StringRegistry string_registry_;
  ArgsSerializer<> serializer_;

  // Cold data - less frequently accessed (separate cache line)
  alignas(base::kCacheSize) SpscQueue queue_;

  // Buffer management (separate cache line)
  alignas(base::LogEntry) alignas(base::kCacheSize) uint8_t
      entry_buffer_[sizeof(base::LogEntry) + kMaxPayloadSize];

  BackendWorker backend_worker_;
  bool terminate_on_fatal_ : 1 = true;
};

inline void InternalLogger::enqueue_log_entry(
    const base::LogEntry* entry) noexcept {
  const size_t entry_size = entry->total_size();

  // Direct enqueue with minimal overhead
  const SpscQueueStatus result = queue_.enqueue_bytes(entry, entry_size);
  if (result == SpscQueueStatus::kOk) [[likely]] {
    enqueued_count_++;
  } else {
    dropped_count_++;
  }

  if (entry->level == base::LogLevel::kFatal && terminate_on_fatal_) {
    backend_worker_.stop();
    std::terminate();
  }
}

}  // namespace femtolog::logging
