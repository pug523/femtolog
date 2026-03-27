// Copyright 2025 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#include "logging/impl/args_serializer.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

#include "base/serialize_util.h"
#include "catch2/catch_test_macros.hpp"

namespace femtolog::logging {

namespace {

TEST_CASE("SerializeBasicTypes", "[ArgsSerializerTest]") {
  const int i = 42;
  const std::string s = "hello";
  const float f = 1.234f;
  const double d = 1.23456789;
  const char* cstr = "world";

  ArgsSerializer<256> serializer;
  auto& args =
      serializer
          .serialize<"int={}, str={}, float={}, double={}, ptr={}", false>(
              i, s, f, d, cstr);

  CHECK(args.size() == sizeof(base::SerializedArgsHeader) + sizeof(i) +
                           sizeof(size_t) + s.size() + sizeof(float) +
                           sizeof(double) + sizeof(size_t) + strlen(cstr));

  auto header =
      reinterpret_cast<const base::SerializedArgsHeader*>(args.data());
  CHECK(header->deserialize_and_format_func != nullptr);
  CHECK(header->format_func != nullptr);

  fmt::memory_buffer buf;
  size_t n = header->deserialize_and_format_func(
      &buf, header->format_func,
      args.data() + sizeof(base::SerializedArgsHeader));

  const char* formatted_str = buf.data();
  CHECK(std::string_view(formatted_str, n) ==
        "int=42, str=hello, float=1.234, double=1.23456789, ptr=world");
}

TEST_CASE("OutOfScope", "[ArgsSerializerTest]") {
  ArgsSerializer<256> serializer;
  base::SerializedArgs<256>* args_ptr = nullptr;

  {
    const int i = 42;
    const char* cstr = "test";

    auto& args = serializer.serialize<"int={}, cstr={}", false>(i, cstr);
    args_ptr = &args;
  }

  CHECK(args_ptr->size() ==
        sizeof(base::SerializedArgsHeader) + sizeof(int) + sizeof(size_t) + 4);

  auto header =
      reinterpret_cast<const base::SerializedArgsHeader*>(args_ptr->data());
  CHECK(header->deserialize_and_format_func != nullptr);
  CHECK(header->format_func != nullptr);

  fmt::memory_buffer buf;
  size_t n = header->deserialize_and_format_func(
      &buf, header->format_func,
      args_ptr->data() + sizeof(base::SerializedArgsHeader));

  const char* formatted_str = buf.data();
  CHECK(std::string_view(formatted_str, n) == "int=42, cstr=test");
}

}  // namespace

}  // namespace femtolog::logging
