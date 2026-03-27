// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

#pragma once

#define FEMTOLOG_BUILD_FLAG_CAT_INDIRECT(a, b) a##b
#define FEMTOLOG_BUILD_FLAG_CAT(a, b) FEMTOLOG_BUILD_FLAG_CAT_INDIRECT(a, b)

#define FEMTOLOG_BUILD_FLAG(flag) \
  (FEMTOLOG_BUILD_FLAG_CAT(FEMTOLOG_BUILD_FLAG_INTERNAL_, flag)())
