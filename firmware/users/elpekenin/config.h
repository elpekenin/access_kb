// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// Having this file on 'users/elpekenin/include' doesnt work, even though it is on path, there's probably something like:
// >>> #include "$(USER_PATH)/config.h"

#pragma once

#include "elpekenin/qp/config.h"
#include "elpekenin/rgb/matrix/config.h"
#include "elpekenin/split/config.h"

#define DYNAMIC_KEYMAP_LAYER_COUNT 5

#define UNICODE_SELECTED_MODES UNICODE_MODE_WINCOMPOSE
#define UCIS_MAX_CODE_POINTS 11
