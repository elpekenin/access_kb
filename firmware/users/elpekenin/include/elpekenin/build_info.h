// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/compiler.h"

#include "generated/features.h"


#define COMMIT_TEXT_LEN 12
typedef struct PACKED {
    char               commit[COMMIT_TEXT_LEN];
    enabled_features_t features;
} build_info_t;

PURE build_info_t get_build_info(void);

NON_NULL(1) READ_ONLY(1) void set_build_commit(const char *commit);
void set_build_features(enabled_features_t features);
