// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define U16_TO_U8(x) ((x) & 0xFF), ((x) >> 8)
#define GET_BIT(x, bit) (((x) >> (bit)) & 1)

// wrap some code such that it doesnt spit logging
// NOTE: silences QMK things, not (at least not granted) `logging` ones
#define WITHOUT_DEBUG(...) \
    do { \
        bool old_debug_state = debug_config.enable; \
        debug_config.enable  = false; \
        __VA_ARGS__ \
        debug_config.enable = old_debug_state; \
    } while (0)
