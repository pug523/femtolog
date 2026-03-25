// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "base/string_util.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <cstring>
#include <cuchar>
#include <queue>
#include <string>
#include <vector>

namespace femtolog::base {

namespace {

void encode_escape_impl(std::string_view input, std::string* out) {
  const size_t max_size = input.size() * 2;  // worst case
  out->clear();
  out->reserve(max_size);

  out->resize(max_size);
  char* write_ptr = out->data();
  char* begin = write_ptr;

  for (char c : input) {
    switch (c) {
      case '\n':
        *write_ptr++ = '\\';
        *write_ptr++ = 'n';
        break;
      case '\r':
        *write_ptr++ = '\\';
        *write_ptr++ = 'r';
        break;
      case '\t':
        *write_ptr++ = '\\';
        *write_ptr++ = 't';
        break;
      case '\\':
        *write_ptr++ = '\\';
        *write_ptr++ = '\\';
        break;
      default: *write_ptr++ = c; break;
    }
  }

  out->resize(static_cast<size_t>(write_ptr - begin));
}

void decode_escape_impl(std::string_view input, std::string* out) {
  const size_t max_size = input.size();  // worst case
  out->clear();
  out->reserve(max_size);

  out->resize(max_size);
  char* write_ptr = out->data();
  char* begin = write_ptr;

  for (size_t i = 0; i < input.size(); i++) {
    if (input[i] == '\\') {
      if (i < input.size() - 1) {
        switch (input[i + 1]) {
          case 'n':
            *write_ptr++ = '\n';
            i++;
            break;
          case 'r':
            *write_ptr++ = '\r';
            i++;
            break;
          case 't':
            *write_ptr++ = '\t';
            i++;
            break;
          case '\\':
            *write_ptr++ = '\\';
            i++;
            break;
          default: *write_ptr++ = input[i]; break;
        }
      } else {
        *write_ptr++ = input[i];
      }
    } else {
      *write_ptr++ = input[i];
    }
  }

  out->resize(static_cast<size_t>(write_ptr - begin));
}

}  // namespace

std::string encode_escape(std::string_view input) {
  std::string encoded;
  encode_escape_impl(input, &encoded);
  return encoded;
}

std::string encode_escape(const char* s, size_t len) {
  return encode_escape(std::string_view(s, len));
}

std::string decode_escape(std::string_view input) {
  std::string decoded;
  decode_escape_impl(input, &decoded);
  return decoded;
}

std::string decode_escape(const char* s, size_t len) {
  return decode_escape(std::string_view(s, len));
}

size_t safe_strlen(const char* str) {
  if (!str) {
    return 0;
  }
  return std::strlen(str);
}

void format_address_safe(uintptr_t addr, char* buffer, size_t buffer_size) {
  if (!buffer || buffer_size < (sizeof(uintptr_t) * 2 + 3)) {
    if (buffer && buffer_size > 0) {
      buffer[0] = '\0';
    }
    return;
  }

  char* current_ptr = buffer;
  current_ptr[0] = '0';
  current_ptr[1] = 'x';
  current_ptr += 2;

  auto [ptr, ec] = std::to_chars(current_ptr, buffer + buffer_size, addr, 16);

  if (ec == std::errc{}) {
    *ptr = '\0';
  } else {
    buffer[0] = '\0';
  }
}

void padding(char*& cursor,
             const char* const end,
             size_t current_len,
             size_t align_len) {
  size_t pad_len = (align_len > current_len) ? align_len - current_len : 0;
  size_t to_pad = std::min(pad_len, static_cast<size_t>(end - cursor));
  std::memset(cursor, ' ', to_pad);
  cursor += to_pad;
}

size_t write_raw(char*& dest, const char* source, size_t len) {
  if (len == 0) {
    return 0;
  }

  std::memcpy(dest, source, len);
  return len;
}

}  // namespace femtolog::base
