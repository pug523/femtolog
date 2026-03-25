// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <sys/stat.h>

#include <cstddef>
#include <string>
#include <vector>

#include "build/build_flag.h"

#if FEMTOLOG_ENABLE_AVX2
#include <immintrin.h>
#endif  // FEMTOLOG_ENABLE_AVX2

namespace femtolog::base {

constexpr const size_t kPathMaxLength = 4096;
// constexpr const size_t kPredictedFilesNbPerDir = 64;
// using Files = std::vector<std::string>;

[[nodiscard]] bool dir_exists(const char* dir_name);
[[nodiscard]] const std::string& exe_path();
[[nodiscard]] const std::string& exe_dir();
[[nodiscard]] std::string parent_dir(const std::string& path);

int create_directory(const char* path);
int create_directories(const char* path);
[[nodiscard]] std::string sanitize_component(const char* part, bool is_first);

template <typename T>
[[nodiscard]] inline std::string to_string_path_part(const T& part) {
  if constexpr (std::is_convertible_v<T, std::string>) {
    return static_cast<std::string>(part);
  } else if constexpr (std::is_convertible_v<T, const char*>) {
    return std::string(part);
  } else {
    static_assert(std::is_same_v<T, void>,
                  "to_string_path_part: unsupported path element type");
  }
}

template <typename... Parts>
[[nodiscard]] std::string join_path(const Parts&... parts) {
  std::string joined;
  joined.reserve(kPathMaxLength);
  bool is_first = true;

  (
      [&] {
        std::string part = to_string_path_part(parts);
        std::string sanitized = sanitize_component(part.c_str(), is_first);
        if (!sanitized.empty()) {
          if (!is_first) {
            joined.push_back(FEMTOLOG_DIR_SEPARATOR);
          }
          joined.append(sanitized);
          is_first = false;
        }
      }(),
      ...);

  return joined;
}

}  // namespace femtolog::base
