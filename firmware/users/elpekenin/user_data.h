// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "generated/features.h"

#define COMMIT_TEXT_LEN 11
typedef struct user_data_t {
    char               commit[COMMIT_TEXT_LEN];
    enabled_features_t features;
} user_data_t;
extern user_data_t user_data;
