// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#define CATCH_CONFIG_RUNNER

#include "base/diagnostics/signal_handler.h"
#include "base/diagnostics/stack_trace.h"
#include "base/diagnostics/terminate_handler.h"
#include "catch2/catch_all.hpp"

void init() {
  femtolog::base::register_signal_handlers();
  femtolog::base::register_terminate_handler();
  femtolog::base::register_stack_trace_handler();
}

void clean_up() {
  // noop
}

int main(int argc, char** argv) {
  init();

  Catch::Session session;

  int return_code = session.applyCommandLine(argc, argv);
  if (return_code != 0) {
    return return_code;
  }

  int result = session.run();

  clean_up();

  return result;
}
