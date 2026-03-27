// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <new>
#include <string>
#include <string_view>

#include "base/check.h"
#include "base/format_util.h"
#include "base/memory_util.h"
#include "base/serialize_util.h"
#include "base/string_registry.h"
#include "build/build_config.h"
#include "logging/impl/args_deserializer.h"

namespace femtolog::logging {

template <bool ref_mode, typename... Args>
consteval size_t calculate_min_serialized_size() {
  size_t total = sizeof(base::SerializedArgsHeader);
  auto add_arg_size = []<typename T>() constexpr -> size_t {
    using Decayed = std::decay_t<T>;
    if constexpr (base::is_string_like_v<Decayed>) {
      if constexpr (ref_mode) {
        return sizeof(size_t) + sizeof(uintptr_t);
      } else {
        return sizeof(size_t);
      }
    } else if constexpr (std::is_trivially_copyable_v<Decayed>) {
      return sizeof(Decayed);
    } else if constexpr (ref_mode) {
      return sizeof(uintptr_t);
    } else {
      static_assert(sizeof(Decayed) == 0,
                    "attempted to write unsupported type\n:"
                    "currently only supporting string like types and trivially "
                    "copyable types");
    }
  };

  ((total += add_arg_size.template operator()<Args>()), ...);
  return total;
}

template <typename T>
inline std::string_view to_string_view(const T& value) {
  using Decayed = std::decay_t<T>;
  static_assert(base::is_string_like_v<Decayed>,
                "cannot convert to string view");
  std::string_view view;
  if constexpr (base::is_char_array_v<Decayed>) {
    if (value == nullptr) [[unlikely]] {
      view = "(nullptr)";
    } else {
      view = value;
    }
  } else {
    view = value;
  }
  return view;
}

template <typename T>
inline void add_dynamic_string_length(size_t* dest, const T& value) {
  using Decayed = std::decay_t<T>;
  if constexpr (base::is_string_like_v<Decayed>) {
    const std::string_view view = to_string_view(value);
    // sizeof(size_t) is already accounted for in the total size by
    // `calculate_min_serialized_size`.
    *dest += view.size();
  }
}

template <bool ref_mode, typename T>
inline void write_arg(char*& pos, const T& value) {
  using Decayed = std::decay_t<T>;

  if constexpr (base::is_string_like_v<Decayed>) {
    if constexpr (ref_mode) {
      const std::string_view view = to_string_view(value);
      const char* cptr = view.data();
      const uintptr_t raw = reinterpret_cast<uintptr_t>(cptr);
      std::memcpy(pos, &raw, sizeof(raw));
      pos += sizeof(raw);

      const size_t str_len = view.size();
      std::memcpy(pos, &str_len, sizeof(str_len));
      pos += sizeof(str_len);
    } else {
      const std::string_view view = to_string_view(value);
      const size_t str_len = view.size();
      std::memcpy(pos, &str_len, sizeof(str_len));
      pos += sizeof(str_len);

      if (str_len > 0) [[likely]] {
        std::memcpy(pos, view.data(), str_len);
        pos += str_len;
      }
    }
  } else if constexpr (std::is_trivially_copyable_v<Decayed>) {
    std::memcpy(pos, &value, sizeof(Decayed));
    pos += sizeof(Decayed);
  } else if constexpr (ref_mode) {
    auto ptr = std::addressof(value);
    std::memcpy(pos, &ptr, sizeof(ptr));
    pos += sizeof(ptr);
  } else {
    static_assert(sizeof(Decayed) == 0,
                  "attempted to serialize unsupported format argument type.");
  }
}

template <size_t kCapacity = 2048>
class ArgsSerializer {
  static_assert(kCapacity >= sizeof(base::SerializedArgsHeader),
                "cannot set kCapacity less than size of header");

 public:
  ArgsSerializer() = default;
  ~ArgsSerializer() = default;

  ArgsSerializer(const ArgsSerializer&) = delete;
  ArgsSerializer& operator=(const ArgsSerializer&) = delete;

  ArgsSerializer(ArgsSerializer&&) noexcept = default;
  ArgsSerializer& operator=(ArgsSerializer&&) noexcept = default;

  inline const base::SerializedArgs<kCapacity>& args() const { return args_; }
  inline base::SerializedArgs<kCapacity>& args() { return args_; }

  // for 0 args: we don't use this
  template <base::FixedString fmt, bool ref_mode>
  inline base::SerializedArgs<kCapacity>& serialize() = delete;

  template <base::FixedString fmt, bool ref_mode, typename... Args>
  [[gnu::hot,
    gnu::always_inline]] inline constexpr base::SerializedArgs<kCapacity>&
  serialize(Args&&... args) {
    constexpr size_t kTotalSizeExcludingDynStr =
        calculate_min_serialized_size<ref_mode, Args...>();
    static_assert(kCapacity >= kTotalSizeExcludingDynStr,
                  "Buffer too small for arguments");

    // header
    constexpr base::FormatFunction format_function_ptr =
        base::FormatDispatcher<fmt>::function();
    constexpr base::DeserializeAndFormatFunction deserialize_function_ptr =
        DeserializeDispatcher<ref_mode, std::decay_t<Args>...>::function();
    constexpr base::SerializedArgsHeader header(format_function_ptr,
                                                deserialize_function_ptr);
    if constexpr (ref_mode) {
      char* pos = args_.data();
      std::memcpy(pos, &header, sizeof(header));
      pos += sizeof(header);

      (write_arg<true>(pos, args), ...);

      args_.resize(kTotalSizeExcludingDynStr);
      return args_;
    } else {
      size_t dynamic_strings_length = 0;
      (add_dynamic_string_length(&dynamic_strings_length, args), ...);

      const size_t total_serialized_size =
          kTotalSizeExcludingDynStr + dynamic_strings_length;
      if (total_serialized_size > kCapacity) {
        args_.resize(0);
        return args_;
      } else {
        char* pos = args_.data();
        std::memcpy(pos, &header, sizeof(header));
        pos += sizeof(header);

        (write_arg<false>(pos, args), ...);

        args_.resize(kTotalSizeExcludingDynStr + dynamic_strings_length);
        return args_;
      }
    }
  }

 private:
  alignas(base::kCacheSize) base::SerializedArgs<kCapacity> args_;
};

// Type aliases
using DefaultSerializer = ArgsSerializer<2048>;
using SmallSerializer = ArgsSerializer<512>;
using LargeSerializer = ArgsSerializer<8192>;

using DefaultSerializedArgs = base::SerializedArgs<2048>;
using SmallSerializedArgs = base::SerializedArgs<512>;
using LargeSerializedArgs = base::SerializedArgs<8192>;

}  // namespace femtolog::logging
