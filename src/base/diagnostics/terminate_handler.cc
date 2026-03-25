// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "base/diagnostics/terminate_handler.h"

#include <iostream>

#include "base/diagnostics/stack_trace.h"

namespace femtolog::base {

void terminate_handler() {
  std::cout << "\nfemtolog: program terminated unexpectedly\n"
            << "stack trace (most recent call last):\n"
            << stack_trace_from_current_context() << "\n";

  std::exit(EXIT_FAILURE);
}

void register_terminate_handler() {
  std::set_terminate(terminate_handler);
}

}  // namespace femtolog::base
