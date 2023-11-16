// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin.h"
#include "graphics.h"
#include "placeholders.h"
#include "user_logging.h"
#include "user_xap.h"

#if defined (RGB_MATRIX_ENABLE)
#    include "user_rgb_functions.h"
#endif // defined (RGB_MATRIX_ENABLE)

#if defined(SPLIT_KEYBOARD)
#    include "transactions.h"
#endif // defined(SPLIT_KEYBOARD)

bool shutdown_user(bool jump_to_bootloader) {
    if (!shutdown_keymap(jump_to_bootloader)) {
        return false;
    }

#if defined(SPLIT_KEYBOARD)
    if (is_keyboard_master()) {
        transaction_rpc_send(RPC_ID_USER_SHUTDOWN, sizeof(jump_to_bootloader), &jump_to_bootloader);
    }
#endif // defined(SPLIT_KEYBOARD)

#if defined(QUANTUM_PAINTER_ENABLE)
    for (uint8_t i = 0; i < QUANTUM_PAINTER_NUM_DISPLAYS; ++i) {
        qp_power(qp_devices_pekenin[i], false);
    }
#endif // defined(QUANTUM_PAINTER_ENABLE)

#if defined(RGB_MATRIX_ENABLE)
    rgb_shutdown(jump_to_bootloader);
#endif // RGB_MATRIX_ENABLE

#if defined(QP_XAP_ENABLE)
    xap_shutdown(jump_to_bootloader);
#endif // defined(QP_XAP_ENABLE)

    return true;
}

static bool suspend_changed     = true;
static bool suspend_debug_state = true;
static bool keyboard_booted     = false;
void suspend_power_down_user(void) {
    // good amount of logic would be lost, so we call housekeeping function while suspended
    if (!suspend_changed) {
        housekeeping_task();
        return;
    }

    // disable debug, saving current setting
    if (keyboard_booted) {
        suspend_debug_state = debug_enable;
        debug_enable        = false;
    }

    // only run suspend-specific code once per suspend duration
    suspend_changed = false;
    suspend_power_down_keymap();
}

void suspend_wakeup_init_user(void) {
    // enable debug after USB is init'ed on startup
    if (!keyboard_booted) {
        keyboard_booted = true;
        debug_enable    = true;
        return;
    }

    // flag the suspend callback to handle code on its next trigger
    suspend_changed = true;

    // restore debug and log event after suspend
    debug_enable = suspend_debug_state;
    logging(UNKNOWN, TRACE, "waking up...");

    suspend_wakeup_init_keymap();
}
