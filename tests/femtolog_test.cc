// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include <string>

#include "catch2/catch_test_macros.hpp"
#include "femtolog/logger.h"
#include "sinks/file_sink.h"
#include "sinks/json_lines_sink.h"
#include "sinks/stdout_sink.h"

TEST_CASE("README Basic Example", "[FemtoLogTest]") {
  // NOLINTNEXTLINE(build/namespaces)
  using namespace femtolog;

  // Initialize the logger
  Logger& logger = Logger::logger();
  logger.init();
  logger.register_sink<femtolog::StdoutSink<>>();
  logger.start_worker();

  // Output "Hello World" to console
  logger.info<"Hello World\n">();

  logger.stop_worker();
  logger.clear_sinks();
}

TEST_CASE("README Advanced Example", "[FemtoLogTest]") {
  // Get thread local logger instance.
  femtolog::Logger& logger = femtolog::Logger::logger();

  // Initialize logger and register log sink.
  femtolog::FemtologOptions options = {
      .spsc_queue_size = 1024 * 1024 * 4,
      .backend_format_buffer_size = 1024 * 64,
      .backend_dequeue_buffer_size = 1024 * 64,
      .backend_worker_cpu_affinity = 5,
      .color_mode = femtolog::ColorMode::kAuto,
  };
  logger.init(options);
  logger.register_sink<femtolog::StdoutSink<>>();
  logger.register_sink<femtolog::FileSink>();
  logger.register_sink<femtolog::JsonLinesSink<>>();
  logger.level("trace");

  // Start the backend worker that dequeues logged entries.
  logger.start_worker();

  std::string username = "pugur";
  float cpu_usage = 42.57f;
  bool result = true;
  int error_code = -1;

  // Log messages with compile-time interpreted format strings.
  logger.trace<"Hello {}\n">("World");
  logger.debug<"Hello World wo formatting\n">();
  logger.info<"User \"{}\" logged in.\n">(username);
  logger.warn<"CPU usage is high: {}%\n">(cpu_usage);
  logger.error<"Return value is: {}\n">(result);

  logger.fatal<"Fatal error occured; error code: {}\n">(error_code);

  logger.stop_worker();
  logger.clear_sinks();
}

namespace femtolog {

TEST_CASE("Basic logging", "[FemtoLogTest]") {
  Logger& logger = Logger::global_logger();

  femtolog::FemtologOptions options{
      .terminate_on_fatal = false,
  };
  logger.init(options);

  logger.register_sink<StdoutSink<>>();
  logger.register_sink<FileSink>();
  logger.register_sink<JsonLinesSink<>>();

  logger.start_worker();

  logger.level(base::LogLevel::kTrace);

  logger.info<"basic logging\n">();

  logger.info<"5 is: {}\n">(5);
  logger.warn<"this is a sample warning, 1.234 is: {}\n">(1.234);
  logger.error<"this is a sample error, true is: {}\n">(true);
  logger.debug<"this is a sample debug, false is: {}\n">(false);
  logger.trace<"this is a sample trace, string is: {}\n">("string");
  std::string msg = "message";
  logger.fatal<"this is a sample fatal {}\n">(msg);

  logger.level(base::LogLevel::kWarn);
  logger.trace<"this line should be invisible1\n">();
  logger.debug<"this line should be invisible2\n">();
  logger.info<"this line should be invisible3\n">();
  logger.level(base::LogLevel::kInfo);

  {
    std::string referenced1 = "test 1";
    std::string referenced2 = "test 2";
    std::string referenced3 = "test 3";
    std::string referenced4 = "test 4";
    logger.info_ref<"reference captured fast logging: {}\n">(referenced1);
    logger.info_ref<"reference captured fast logging: {}\n">(referenced2);
    logger.info_ref<"reference captured fast logging: {}\n">(referenced3);
    logger.info_ref<"reference captured fast logging: {}\n">(referenced4);
    logger.flush();
  }

  logger.stop_worker();
  logger.clear_sinks();
}

}  // namespace femtolog
