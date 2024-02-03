// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include "elpekenin.h"
#include "elpekenin/rng.h"
#include "elpekenin/xap.h"
#include "elpekenin/qp/graphics.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_QWERTY] = LAYOUT(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,           KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,           KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    ES_PLUS,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,           KC_H,    KC_J,    KC_K,    KC_L,    TD_NTIL, KC_ENT,
        KC_LSFT, TD_Z,    KC_X,    KC_C,    KC_V,    KC_B,           KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_UP,   KC_VOLU,
        KC_LCTL, KC_LGUI, TL_UPPR, KC_LALT,     TD_SPC,                  R_SPC,        TL_LOWR, KC_LEFT, KC_DOWN, KC_RIGHT
    ),

    // LOWER
    [_FN1] = LAYOUT(
        XXXXXXX, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,          KC_F6,   KC_F7,   KC_F8,   KC_9,    KC_F10,  ES_BSLS,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, ES_LBRC, ES_RBRC, PK_CPYR,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, RGB_VAI, RGB_MOD,
        XXXXXXX, XXXXXXX, _______, XXXXXXX,    PK_UCIS,                  XXXXXXX,      _______, RGB_SPD, RGB_VAD, RGB_SPI
    ),

    // UPPER
    // Note: Using number row keycodes instead of numpad, so we dont care about numlock
    [_FN2] = LAYOUT(
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        KC_7,    KC_8,    KC_9,    XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        KC_4,    KC_5,    KC_6,    XXXXXXX, XXXXXXX, XXXXXXX,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        KC_1,    KC_2,    KC_3,    XXXXXXX, RGB_VAI, XXXXXXX,
        XXXXXXX, XXXXXXX, _______, XXXXXXX,     XXXXXXX,                  KC_0,        _______, RGB_SPD, RGB_VAD, RGB_SPI
    ),

    [_FN3] = LAYOUT(
        XXXXXXX, ES_PIPE, ES_AT,   ES_HASH, ES_TILD, ES_EURO,        ES_NOT,  XXXXXXX, XXXXXXX, XXXXXXX, ES_QUOT, ES_BSLS,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, TD_GRV,  XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, ES_LCBR, ES_RCBR, XXXXXXX,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, ES_MINS, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,      XXXXXXX,                 _______,     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX
    ),

    // ADJUST
    [_RST] = LAYOUT(
        QK_BOOT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, PK_LOG,         PK_GAME, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, EE_CLR,
        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        PK_QCLR, AC_TOGG, XXXXXXX, XXXXXXX, PK_SIZE, PK_PCSH,        PK_KLOG, PK_CRSH, XXXXXXX, XXXXXXX, XXXXXXX, QK_RBT,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, _______, XXXXXXX,     DB_TOGG,                 DB_TOGG,      _______, XXXXXXX, XXXXXXX, AC_DICT
    ),
};

const char *fmt = "%M\n";
void keyboard_pre_init_keymap(void) {
    set_logging_fmt(fmt);
}

#if defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && IS_RIGHT_HAND
static uint32_t read_touch_callback(uint32_t trigger_time, void *cb_arg) {
    uint32_t interval = TOUCH_MS;

    // Do nothing until sensor initialised or when screen isn't pressed
    if (!is_ili9341_pressed()) {
        xap_screen_released(ILI9341_ID);
        return interval;
    }

    // Make a read and send it to Tauri
    touch_report_t ili9341_touch_report = get_spi_touch_report(ili9341_touch, false);

    xap_screen_pressed(ILI9341_ID, ili9341_touch_report);

    return interval;
}
#endif

void keyboard_post_init_keymap(void) {
#if defined(QUANTUM_PAINTER_ENABLE)
#    if IS_LEFT_HAND
    load_display(il91874);
#    endif

#    if IS_RIGHT_HAND
    load_display(ili9163);
    load_display(ili9341);

    set_uptime_device(ili9341);
    set_logging_device(ili9341);
    set_heap_stats_device(ili9341);
    set_layer_device(ili9341);
#        if defined(KEYLOG_ENABLE)
    set_keylog_device(ili9341);
#        endif

#        if defined (TOUCH_SCREEN_ENABLE)
    defer_exec(10, read_touch_callback, NULL);
#        endif

#    endif
#endif

    rng_set_seed(analogReadPin(GP28) * analogReadPin(GP28));
}

void build_info_sync_keymap_callback(void) {
#if IS_LEFT_HAND && defined(QUANTUM_PAINTER_ENABLE)
    draw_commit(il91874);
    draw_features(il91874);
#endif
}
