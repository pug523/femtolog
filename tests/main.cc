#define CATCH_CONFIG_RUNNER

#include "catch2/catch_all.hpp"
#include "femtolog/core/diagnostics/signal_handler.h"
#include "femtolog/core/diagnostics/stack_trace.h"
#include "femtolog/core/diagnostics/terminate_handler.h"

void init() {
  femtolog::core::register_signal_handlers();
  femtolog::core::register_terminate_handler();
  femtolog::core::register_stack_trace_handler();
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
