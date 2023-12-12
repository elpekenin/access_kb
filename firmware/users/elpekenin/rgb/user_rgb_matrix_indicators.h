// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "color.h"

// *********
// * Types *
// *********

typedef struct indicator_t indicator_t;

// arguments passed from rgb_matrix_indicators to indicators_fn_t
typedef struct {
    uint8_t  layer;
    uint8_t  mods;
    uint16_t keycode;
} indicator_fn_args_t;

typedef bool(* indicator_fn_t)(indicator_t *self, indicator_fn_args_t *args);

// indicator specification: condition when it has to be drawn + color
struct indicator_t {
    // common config
    rgb_led_t      color;
    indicator_fn_t check;

    // conditions
    uint8_t  mods;
    uint8_t  layer;
    uint16_t keycode;
};

// **********************
// * Internal functions *
// **********************

bool keycode_callback(indicator_t *indicator, indicator_fn_args_t *args);
bool layer_callback(indicator_t *indicator, indicator_fn_args_t *args);
bool keycode_and_layer_callback(indicator_t *indicator, indicator_fn_args_t *args);
bool layer_and_configured_callback(indicator_t *indicator, indicator_fn_args_t *args);
bool keycode_and_mods_callback(indicator_t *indicator, indicator_fn_args_t *args);

// **********
// * Macros *
// **********

// this crap is needed for RGB color macros to work nicely...
// relying on position aka: (rgb_led_t){_col} doesnt work
#define _RGB(_r, _g, _b) (rgb_led_t){ \
    .r = _r, \
    .g = _g, \
    .b = _b, \
}

#define KC(_kc, _col) { \
    .keycode = _kc, \
    .color = _RGB(_col), \
    .check = &keycode_callback, \
}

#define LAYER(_layer, _col) { \
    .color = _RGB(_col), \
    .check = &layer_callback, \
    .layer = _layer, \
}

#define KC_LAYER(_kc, _layer, _col) { \
    .keycode = _kc, \
    .color = _RGB(_col), \
    .check = &keycode_and_layer_callback, \
    .layer = _layer, \
}

#define NO_TRNS(_layer, _col) { \
    .color = _RGB(_col), \
    .check = &layer_and_configured_callback, \
    .layer = _layer, \
}

#define KC_MOD(_kc, _mods, _col) { \
    .keycode = _kc, \
    .color = _RGB(_col), \
    .check = &keycode_and_mods_callback, \
    .mods = _mods, \
}

// **********
// * Colors *
// **********

#define MAX_WHITE RGB_MATRIX_MAXIMUM_BRIGHTNESS, RGB_MATRIX_MAXIMUM_BRIGHTNESS, RGB_MATRIX_MAXIMUM_BRIGHTNESS

enum colors {
    RED = 0,
    ORANGE = 21,
    YELLOW = 43,
    CHARTREUSE = 64,
    GREEN = 85,
    SPRING = 106,
    CYAN = 127,
    AZURE = 148,
    BLUE = 169,
    VIOLET = 180,
    MAGENTA = 201,
    ROSE = 222,

    // special values
    TRNS = 253,
    WHITE = 254,
    BLACK = 255,

    // has to be equal to the smallest special value
    // ... or rework the `is_special_color` function
    _MARKER_ = TRNS,
};

// *******
// * API *
// *******

bool draw_indicators(uint8_t led_min, uint8_t led_max);
