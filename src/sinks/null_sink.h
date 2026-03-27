// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstddef>

#include "base/log_entry.h"
#include "sinks/sink_base.h"

namespace femtolog {

class NullSink final : public SinkBase {
 public:
  NullSink() = default;
  ~NullSink() override = default;

  inline void on_log(const base::LogEntry&, const char*, size_t) override {
    // noop
  };
};

}  // namespace femtolog
