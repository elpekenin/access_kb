// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "action_layer.h"
#include "action_util.h"
#include "default_keyboard.h" // for LAYOUT
#include "keymap_common.h"
#include "progmem.h"

#include "elpekenin.h" // layers names and custom keycodes
#include "elpekenin/rgb/matrix/indicators.h"
#include "elpekenin/utils/compiler.h"


// *** Definitions ***

static const indicator_t indicators[] = {
    LAYER(_RST, RGB_OFF),

    // QMK keycodes
    KC_LAYER(QK_BOOT, _RST, RGB_RED),
    KC_LAYER(QK_RBT,  _RST, RGB_RED),
    KC_LAYER(EE_CLR,  _RST, RGB_RED),
    KC_LAYER(DB_TOGG, _RST, RGB_RED),
    KC_LAYER(AC_DICT, _RST, RGB_RED),

    // custom keycodes
    KC_CUSTOM_LAYER(_RST, RGB_BLUE)
};

// NOTES:
//   - Assumes (for now?) that all LEDs are mapped to a key (no underglow or w/e)
//   - Available colors defined on the `.h` file
//   - TRNS on layer 0 => nothing drawn (respects animation)
//   - Undefined layers => same as above
static const uint8_t PROGMEM ledmap[][MATRIX_ROWS][MATRIX_COLS] = {
    // [_QWERTY] = LAYOUT(
    //     RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
    //     RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
    //     RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
    //     RED,  RED,  RED,  RED,  RED,  RED,     RED,  RED,  RED,  RED,  RED,  RED,
    //     RED,  RED,  RED,  RED,    BLACK,         WHITE,    RED,  TRNS, RED,  RED
    // ),
    // [_FN1] = LAYOUT(
    //     TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,    TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,
    //     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN,    CYAN, CYAN, CYAN, CYAN, CYAN, CYAN,
    //     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,    BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
    //     ROSE, ROSE, ROSE, ROSE, ROSE, ROSE,    ROSE, ROSE, ROSE, ROSE, ROSE, ROSE,
    //     WHITE,WHITE,BLACK,TRNS,    BLACK,         BLACK,   RED,  TRNS, WHITE,WHITE
    // )
};


// *** Checks ***

// draw the given keycode
RGB_INDICATOR_FN_ATTRS bool keycode_callback(indicator_t *indicator, indicator_fn_args_t *args) {
    return indicator->keycode == args->keycode;
}

// draw every key while on the given layer
RGB_INDICATOR_FN_ATTRS bool layer_callback(indicator_t *indicator, indicator_fn_args_t *args) {
    return indicator->layer == args->layer;
}

// draw every custom keycode on the given layer
RGB_INDICATOR_FN_ATTRS bool custom_keycode_layer_callback(indicator_t *indicator, indicator_fn_args_t *args) {
    return (
        indicator->layer == args->layer
        && args->keycode >= __CUSTOM_KEYCODES_START
    );
}

// draw the given keycode while on the given layer
RGB_INDICATOR_FN_ATTRS bool keycode_and_layer_callback(indicator_t *indicator, indicator_fn_args_t *args) {
    return (
        indicator->keycode == args->keycode
        && indicator->layer == args->layer
    );
}

// draw every keycode configured (i.e. not KC_NO nor KC_TRNS) on the given layer
RGB_INDICATOR_FN_ATTRS bool layer_and_configured_callback(indicator_t *indicator, indicator_fn_args_t *args) {
    return (
        indicator->layer == args->layer
        && indicator->keycode > KC_TRNS
    );
}

// draw the given keycode if given mods are set (i.e. display shortcuts)
RGB_INDICATOR_FN_ATTRS bool keycode_and_mods_callback(indicator_t *indicator, indicator_fn_args_t *args) {
    return (
        indicator->keycode == args->keycode
        && indicator->mods == args->mods
    );
}

// *** Ledmap ***

#define LEDMAP_LAYERS (sizeof(ledmap) / (MATRIX_ROWS * MATRIX_COLS))

CONST static inline bool is_special_color(uint8_t hue) {
    return hue >= _MARKER_;
};

NON_NULL(4) WRITE_ONLY(4) static inline bool get_ledmap_color(uint8_t layer, uint8_t row, uint8_t col, rgb_led_t *rgb) {
    if (layer >= LEDMAP_LAYERS) {
        return false;
    }

    uint8_t hue = pgm_read_byte(&(ledmap[layer][row][col]));
    uint8_t sat = rgb_matrix_get_sat();
    uint8_t val = rgb_matrix_get_val();
    HSV hsv = {hue, sat, val};

    // not "regular" colors (hue), but ones with special handling
    if (is_special_color(hue)) {
        switch (hue) {
            case TRNS:
                if (layer == 0) {
                    return false;
                }
                return get_ledmap_color(layer - 1, row, col, rgb);

            case WHITE:
                hsv = (HSV){0, 0, val};
                break;

            case BLACK:
                hsv = (HSV){0, 0, 0};
                break;
        }
    }

    *rgb = hsv_to_rgb(hsv);
    return true;
}


// *** Callback ***

bool draw_indicators(uint8_t led_min, uint8_t led_max) {
    uint8_t mods  = get_mods();
    uint8_t layer = get_highest_layer(layer_state);

    indicator_fn_args_t args = {
        .mods = mods,
        .layer = layer,
    };

    // iterate all keys
    for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
        for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
            uint8_t index = g_led_config.matrix_co[row][col];

            // early exit if out of range
            if (index < led_min || index >= led_max) {
                continue;
            }

            // draw ledmap color
            rgb_led_t rgb;
            if (get_ledmap_color(layer, row, col, &rgb)) {
                rgb_matrix_set_color(index, rgb.r, rgb.g, rgb.b);
            }

            args.keycode = keymap_key_to_keycode(layer, (keypos_t){col,row});

            // iterate all indicators
            for (
                const indicator_t *indicator = indicators;
                indicator < &indicators[ARRAY_SIZE(indicators)];
                ++indicator
            ) {
                // if check passed, draw
                if (indicator->check((indicator_t *)indicator, &args)) { // cast to discard const
                    rgb_matrix_set_color(index, indicator->color.r, indicator->color.g, indicator->color.b);
                }
            }

        }
    }

    return false;
}