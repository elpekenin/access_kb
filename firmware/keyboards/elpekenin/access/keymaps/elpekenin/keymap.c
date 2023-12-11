// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#include "qp_surface.h"

#include "elpekenin.h"
#include "graphics.h"
#include "rng.h"
#include "user_xap.h"


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
        PK_QCLR, AC_TOGG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        PK_KLOG, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, QK_RBT,
        _______, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,        XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
        XXXXXXX, XXXXXXX, _______, XXXXXXX,     DB_TOGG,                 DB_TOGG,      _______, XXXXXXX, XXXXXXX, XXXXXXX
    ),
};

// #if !defined(INIT_EE_HANDS_LEFT)
// static uint8_t framebuffer[SURFACE_REQUIRED_BUFFER_BYTE_SIZE(ILI9163_WIDTH, ILI9163_HEIGHT, 16)];
// static painter_device_t surface;
// #endif

void keyboard_post_init_keymap(void) {
#if defined(QUANTUM_PAINTER_ENABLE)
#    if defined(INIT_EE_HANDS_LEFT)
    load_display(il91874);
    qp_log_target_device = NULL;
#    else
    load_display(ili9163);
    load_display(ili9341);
    qp_log_target_device = ili9341;

    // // transparent text test
    // surface = qp_make_rgb565_surface(ILI9163_WIDTH, ILI9163_HEIGHT, framebuffer);
    // qp_init(surface, QP_ROTATION_0);
    // qp_rect(surface, 30, 30, 80, 80, HSV_RED, true);
    // qp_drawtext(surface, 30, 40, qp_fonts[2], "Testing");
    // qp_surface_draw(surface, ili9163, 0, 0, true);
#    endif // defined(INIT_EE_HANDS_LEFT)
#endif // defined(QUANTUM_PAINTER_ENABLE)

    rng_set_seed(analogReadPin(GP28) * analogReadPin(GP28));
}

void user_data_sync_keymap_callback(void) {
#if defined(INIT_EE_HANDS_LEFT) && defined(QUANTUM_PAINTER_ENABLE)
    draw_commit(il91874);
    draw_features(il91874);
#endif // defined(INIT_EE_HANDS_LEFT) && defined(QUANTUM_PAINTER_ENABLE)
}

#if defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)
void housekeeping_task_keymap(void) {
    uint32_t now = timer_read32();

    static uint32_t touch_timer = 0;

    // We only read once in a while
    if (TIMER_DIFF_32(now, touch_timer) < TOUCH_MS)
        return;

    touch_timer = now;

    // Do nothing until sensor initialised or when screen isn't pressed
    if (!ili9341_touch || !ili9341_pressed) {
        xap_screen_released(ILI9341_ID);
        return;
    }

    // Make a read and send it to Tauri
    touch_report_t ili9341_touch_report = get_spi_touch_report(ili9341_touch, false);

    xap_screen_pressed(ILI9341_ID, ili9341_touch_report);

#    if defined(ONE_HAND_ENABLE)
    screen_one_hand(ili9341_touch_report);
#    endif // ONE_HAND_ENABLE
}
#endif // defined(QUANTUM_PAINTER_ENABLE) && defined (TOUCH_SCREEN_ENABLE) && defined(INIT_EE_HANDS_RIGHT)

const char *fmt = "%M\n";
void keyboard_pre_init_keymap(void) {
    set_logging_fmt(fmt);
}