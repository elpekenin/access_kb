// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"

#include "elpekenin/xap.h"
#include "elpekenin/utils/compiler.h"

#include "generated/keycode_str.h"


#if defined(TOUCH_SCREEN_ENABLE)
void xap_screen_pressed(uint8_t screen_id, touch_report_t report) {
    screen_pressed_msg_t msg = {
        .msg_id = _SCREEN_PRESSED,
        .screen_id = screen_id,
        .x = report.x,
        .y = report.y
    };

    xap_broadcast_user(&msg, sizeof(msg));
}

void xap_screen_released(uint8_t screen_id) {
    screen_released_msg_t msg = {
        .msg_id = _SCREEN_RELEASED,
        .screen_id = screen_id
    };

    xap_broadcast_user(&msg, sizeof(msg));
}
#endif // defined(TOUCH_SCREEN_ENABLE)

void xap_layer(layer_state_t state) {
    layer_change_msg_t msg = {
        .msg_id = _LAYER_CHANGE,
        .layer = get_highest_layer(state)
    };

    xap_broadcast_user(&msg, sizeof(msg));
}

void xap_keyevent(uint16_t keycode, keyrecord_t *record) {
    keyevent_msg_t msg = {
        .base.msg_id = _KEYEVENT,
        .base.keycode = keycode,
        .base.pressed = record->event.pressed,
        .base.layer = get_highest_layer(layer_state),
        .base.row = record->event.key.row,
        .base.col = record->event.key.col,
        .base.mods = get_mods()
    };

    strcpy(msg.str, get_keycode_str_at(msg.base.layer, msg.base.row, msg.base.col));

    xap_broadcast_user(&msg, sizeof(msg));
}

void xap_shutdown(bool jump_to_bootloader) {
    shutdown_msg_t msg = {
        .msg_id = _SHUTDOWN,
        .bootloader = jump_to_bootloader,
    };

    xap_broadcast_user(&msg, sizeof(msg));
}
