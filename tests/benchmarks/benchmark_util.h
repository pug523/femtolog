// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <string>

#include "femtolog/base/file_util.h"

namespace femtolog::bench {

inline std::string get_benchmark_log_path(const char* filename) {
  return base::join_path("benchmark_logs", filename);
}

}  // namespace femtolog::bench
