// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "femtolog/logging/impl/internal_logger.h"

#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "catch2/catch_all.hpp"
#include "femtolog/base/log_entry.h"
#include "femtolog/base/log_level.h"
#include "femtolog/options.h"
#include "femtolog/sinks/sink_base.h"

namespace femtolog::logging {

using base::FixedString;
using base::LogEntry;
using base::LogLevel;

class SpySink : public SinkBase {
 public:
  struct LogData {
    uint32_t thread_id;
    uint16_t format_id;
    LogLevel level;
    std::string message;
    uint64_t timestamp_ns;
  };

  void on_log(const LogEntry& entry, const char* content, size_t len) override {
    std::lock_guard<std::mutex> lock(mtx_);
    LogData data;
    data.thread_id = entry.thread_id;
    data.format_id = entry.format_id;
    data.level = entry.level;
    data.message = std::string(content, len);
    data.timestamp_ns = entry.timestamp_ns;
    captured_logs.push_back(std::move(data));
    call_count++;
  }

  size_t get_call_count() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return call_count;
  }

  std::vector<LogData> get_logs() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return captured_logs;
  }

  void clear() {
    std::lock_guard<std::mutex> lock(mtx_);
    captured_logs.clear();
    call_count = 0;
  }

 private:
  mutable std::mutex mtx_;
  std::vector<LogData> captured_logs;
  size_t call_count = 0;
};

TEST_CASE("InternalLogger basic functionality", "[InternalLoggerTest]") {
  auto logger = std::make_unique<InternalLogger>();
  auto spy_sink = std::make_unique<SpySink>();
  auto* spy_ptr = spy_sink.get();

  SECTION("Basic initialization") {
    logger->init();
    CHECK(logger->level() == LogLevel::kInfo);
    CHECK(logger->enqueued_count() == 0);
    CHECK(logger->dropped_count() == 0);
    CHECK(logger->thread_id() != 0);
  }

  SECTION("Sink registration and clearing") {
    logger->init();
    logger->register_sink(std::move(spy_sink));
    CHECK(spy_sink == nullptr);
    logger->clear_sinks();
  }

  SECTION("Log level setting") {
    logger->init();
    logger->level(LogLevel::kDebug);
    CHECK(logger->level() == LogLevel::kDebug);
    logger->level(LogLevel::kError);
    CHECK(logger->level() == LogLevel::kError);
  }

  SECTION("Literal string logging") {
    logger->init();
    logger->register_sink(std::move(spy_sink));
    logger->start_worker();

    constexpr FixedString msg = "Test message";
    logger->log<base::LogLevel::kInfo, msg, false>();

    logger->stop_worker();

    auto logs = spy_ptr->get_logs();
    REQUIRE(logs.size() == 1);
    CHECK(logs[0].level == LogLevel::kInfo);
    CHECK(logs[0].format_id == base::kLiteralLogStringId);
    CHECK(logs[0].message == "Test message");
  }

  SECTION("Parameterized logging") {
    logger->init();
    logger->register_sink(std::move(spy_sink));
    logger->start_worker();

    constexpr FixedString fmt = "Value: {}";
    logger->log<base::LogLevel::kInfo, fmt, false>(42);

    logger->stop_worker();

    auto logs = spy_ptr->get_logs();
    REQUIRE(logs.size() == 1);
    CHECK(logs[0].level == LogLevel::kInfo);
    CHECK(logs[0].format_id != base::kLiteralLogStringId);
  }

  SECTION("Log level filtering") {
    logger->init();
    logger->level(LogLevel::kWarn);
    logger->register_sink(std::move(spy_sink));
    logger->start_worker();

    // Filtered
    constexpr FixedString debug_msg = "Debug message";
    logger->log<base::LogLevel::kDebug, debug_msg, false>();

    // Allowed
    constexpr FixedString error_msg = "Error message";
    logger->log<base::LogLevel::kError, error_msg, false>();

    logger->stop_worker();

    auto logs = spy_ptr->get_logs();
    REQUIRE(logs.size() == 1);
    CHECK(logs[0].level == LogLevel::kError);
  }

  SECTION("Counter and thread ID consistency") {
    logger->init();
    logger->register_sink(std::move(spy_sink));
    logger->start_worker();

    uint64_t initial_enqueued = logger->enqueued_count();
    uint32_t logger_thread_id = logger->thread_id();

    constexpr FixedString msg = "Consistency test";
    logger->log<base::LogLevel::kInfo, msg, false>();

    logger->stop_worker();

    CHECK(logger->enqueued_count() > initial_enqueued);
    auto logs = spy_ptr->get_logs();
    REQUIRE(logs.size() == 1);
    CHECK(logs[0].thread_id == logger_thread_id);
  }

  SECTION("Realistic integration scenario") {
    logger->init();
    logger->level(LogLevel::kDebug);
    logger->register_sink(std::move(spy_sink));
    logger->start_worker();

    constexpr FixedString startup_msg = "Application starting";
    constexpr FixedString process_fmt = "Processing item: {}";
    constexpr FixedString complete_msg = "Processing complete";

    logger->log<base::LogLevel::kInfo, startup_msg, false>();
    for (int i = 0; i < 3; ++i) {
      logger->log<base::LogLevel::kDebug, process_fmt, false>(i);
    }
    logger->log<base::LogLevel::kInfo, complete_msg, false>();

    logger->stop_worker();

    auto logs = spy_ptr->get_logs();
    REQUIRE(logs.size() >= 5);

    for (size_t i = 1; i < logs.size(); ++i) {
      CHECK(logs[i].timestamp_ns >= logs[i - 1].timestamp_ns);
    }
  }
}

}  // namespace femtolog::logging
