// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <format>
#include <queue>
#include <string>
#include <string_view>
#include <utility>

namespace femtolog::base {

[[nodiscard]] std::string encode_escape(std::string_view input);
[[nodiscard]] std::string encode_escape(const char* s, size_t len);
[[nodiscard]] std::string decode_escape(std::string_view input);
[[nodiscard]] std::string decode_escape(const char* s, size_t len);

[[nodiscard]] size_t safe_strlen(const char* str);

void format_address_safe(uintptr_t addr,
                         char* buffer_start,
                         size_t buffer_size);

void padding(char*& cursor,
             const char* const end,
             size_t current_len,
             size_t align_len);

size_t write_raw(char*& dest, const char* source, size_t len);

template <typename... Args>
size_t write_format(char*& cursor,
                    const char* const end,
                    std::format_string<Args...> fmt,
                    Args&&... args) {
  std::ptrdiff_t remaining = end - cursor;
  if (remaining <= 0) {
    return 0;
  }

  const auto result =
      std::format_to_n(cursor, remaining - 1, fmt, std::forward<Args>(args)...);
  const size_t written = static_cast<size_t>(result.out - cursor);

  cursor += written;
  *cursor = '\0';
  return written;
}

template <typename T>
struct is_ostreamable {
 private:
  template <typename U>
  static auto test(int)  // NOLINT
      -> decltype(std::declval<std::ostream&>() << std::declval<U>(),
                  std::true_type{});

  template <typename>
  static std::false_type test(...);

 public:
  static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename T>
inline constexpr bool is_ostreamable_v = is_ostreamable<T>::value;

}  // namespace femtolog::base
