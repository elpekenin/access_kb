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

// wrap the above, to also silence logging for a "feature"
#define WITHOUT_LOGGING(feature, ...) \
    do { \
        log_level_t old_level = get_level_for(feature); \
        set_level_for(feature, LOG_NONE); \
        WITHOUT_DEBUG(__VA_ARGS__); \
        set_level_for(feature, old_level); \
    } while (0)

#define WIPE_VAR(__x) memset(&(__x), 0, sizeof(__x))
#define WIPE_ARRAY(__x) memset((__x), 0, sizeof(__x))

#define HIGHEST_LAYER (get_highest_layer(layer_state | default_layer_state))