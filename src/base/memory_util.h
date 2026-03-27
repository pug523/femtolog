// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "build/build_config.h"

#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
#include <windows.h>
#elif FEMTOLOG_BUILD_FLAG(IS_OS_LINUX)
#include <sys/mman.h>
#endif

namespace femtolog::base {

// constexpr size_t kPrefetchThreshold = 256;

// Optimized memory allocation with proper alignment
inline void* aligned_alloc_wrapper(size_t alignment, size_t size) {
#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
  return _aligned_malloc(size, alignment);
#else
  void* ptr = nullptr;
  if (posix_memalign(&ptr, alignment, size) != 0) {
    return nullptr;
  }
  return ptr;
#endif
}

inline void aligned_free_wrapper(void* ptr) {
#if FEMTOLOG_BUILD_FLAG(IS_OS_WIN)
  _aligned_free(ptr);
#else
  free(ptr);
#endif
}

struct AlignedDeleter {
  inline void operator()(void* ptr) const { aligned_free_wrapper(ptr); }
};

#ifdef __GCC_DESTRUCTIVE_SIZE
constexpr const size_t kCacheSize = __GCC_DESTRUCTIVE_SIZE;
#else
constexpr const size_t kCacheSize = 64;
#endif

}  // namespace femtolog::base
