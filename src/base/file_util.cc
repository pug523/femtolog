// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "base/file_util.h"

#include <limits.h>

#include <chrono>
#include <utility>

#include "base/check.h"
#include "build/build_flag.h"

#if FEMTOLOG_IS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#undef APIENTRY
#include <io.h>
#include <windows.h>
#else
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

namespace femtolog::base {

bool dir_exists(const char* dir_name) {
#if FEMTOLOG_IS_WINDOWS
  DWORD attributes = GetFileAttributesA(dir_name);
  return (attributes != INVALID_FILE_ATTRIBUTES &&
          (attributes & FILE_ATTRIBUTE_DIRECTORY));
#else
  struct stat buffer;
  return stat(dir_name, &buffer) == 0 && S_ISDIR(buffer.st_mode);
#endif
}

const std::string& exe_path() {
  static const std::string cached_path = []() -> std::string {
#if FEMTOLOG_IS_WINDOWS
    char path[MAX_PATH] = {};
    DWORD len = GetModuleFileNameA(nullptr, path, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) {
      return "";
    }
    return (len == 0 || len >= MAX_PATH) ? "" : std::string(path, len);
#else
    char path[kPathMaxLength] = {};
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    return len == -1 ? "" : std::string(path, static_cast<size_t>(len));
#endif
  }();

  return cached_path;
}

const std::string& exe_dir() {
  static const std::string cached_dir = []() -> std::string {
    const std::string& this_exe_path = exe_path();
    if (this_exe_path.empty()) {
      return "";
    }

    return parent_dir(this_exe_path);
  }();
  return cached_dir;
}

std::string parent_dir(const std::string& path) {
  size_t pos = path.find_last_of(FEMTOLOG_DIR_SEPARATOR);
  if (pos == std::string::npos) {
    return "";
  }
  return path.substr(0, pos);
}

int create_directory(const char* path) {
#if FEMTOLOG_IS_WINDOWS
  BOOL ok = CreateDirectoryA(path, nullptr);
  if (!ok && GetLastError() != ERROR_ALREADY_EXISTS) {
    return -1;
  }
  return 0;
#else
  return mkdir(path, 0755) == 0 || errno == EEXIST ? 0 : -1;
#endif
}

int create_directories(const char* path) {
  std::string sanitized_path = sanitize_component(path, true);
  size_t pos = 0;
  int result = 0;

  while (pos < sanitized_path.size()) {
    size_t next_pos = sanitized_path.find(FEMTOLOG_DIR_SEPARATOR, pos);
    std::string dir;

    if (next_pos != std::string::npos) {
      dir = sanitized_path.substr(0, next_pos);
      pos = next_pos + 1;
    } else {
      dir = sanitized_path;
      pos = sanitized_path.size();
    }

    if (!dir.empty() && !dir_exists(dir.c_str())) {
      if (create_directory(dir.c_str()) != 0) {
        std::cerr << "Failed to create directory: " << dir << "\n";
        result++;
      }
    }
  }

  return result;
}

std::string sanitize_component(const char* part, bool is_first) {
  std::string result = part;

  // Remove leading separator if not first
  if (!is_first && !result.empty() &&
      result.front() == FEMTOLOG_DIR_SEPARATOR) {
    result.erase(0, 1);
  }

  // Remove trailing separator
  if (!result.empty() && result.back() == FEMTOLOG_DIR_SEPARATOR) {
    result.pop_back();
  }

  return result;
}

}  // namespace femtolog::base
