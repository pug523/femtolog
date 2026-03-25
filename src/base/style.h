// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstddef>

namespace femtolog::base {

constexpr const char* kReset = "\033[0m";

// Logging
constexpr const char* kTraceStyle = "\033[1;38;2;064;064;064m";  // Gray
constexpr const char* kDebugStyle = "\033[1;38;2;040;190;240m";  // Blue
constexpr const char* kInfoStyle = "\033[1;38;2;050;230;050m";   // Green
constexpr const char* kWarnStyle = "\033[1;38;2;245;220;015m";   // Yellow
constexpr const char* kErrorStyle = "\033[1;38;2;255;005;005m";  // Red
constexpr const char* kFatalStyle = "\033[1;38;2;255;040;255m";  // Magenta

// Length of Log Level ANSI style string
constexpr size_t kAnsiStyleLen = 21;

}  // namespace femtolog::base
