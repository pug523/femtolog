// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstddef>
#include <string>

#include "femtolog/build/build_flag.h"

namespace femtolog::base {

struct StackTraceEntry;

static constexpr size_t kDefaultFirstFrame = 1;

#if FEMTOLOG_IS_WINDOWS
static constexpr size_t kPlatformMaxFrames = 62;
#elif FEMTOLOG_IS_MAC
static constexpr size_t kPlatformMaxFrames = 128;
#elif FEMTOLOG_IS_LINUX
static constexpr size_t kPlatformMaxFrames = 64;
#else
static constexpr size_t kPlatformMaxFrames = 64;
#endif

constexpr const size_t kLineBufferSize = 1024;
constexpr const size_t kSymbolBufferSize = 512;
constexpr const size_t kDemangledBufferSize = 512;
constexpr const size_t kMangledBufferSize = 256;

// Example stack trace output:
//
// @0     0x123456789abc   bar()        at /path/to/file
// @1     0x23456789abcd   foo()        at /path/to/file
// @2     0x3456789abcde   hoge()       at /path/to/file
// @3     0x456789abcdef   fuga()       at /path/to/file

[[nodiscard]] std::string stack_trace_from_current_context(
    bool use_index = true,
    size_t first_frame = kDefaultFirstFrame,
    size_t max_frames = kPlatformMaxFrames);

void stack_trace_from_current_context(char* buffer,
                                      size_t buffer_size,
                                      bool use_index = true,
                                      size_t first_frame = kDefaultFirstFrame,
                                      size_t max_frames = kPlatformMaxFrames);

void register_stack_trace_handler();

}  // namespace femtolog::base
