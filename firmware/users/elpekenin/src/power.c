// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "debug.h"

#include "elpekenin.h"
#include "elpekenin/logging.h"
#include "elpekenin/placeholders.h"
#include "elpekenin/utils/deinit.h"


// positions of the first and last entries
// each entry is a pointer to an init func
extern deinit_fn __elpekenin_deinit_base__,
                 __elpekenin_deinit_end__;

bool shutdown_user(bool jump_to_bootloader) {
    if (!shutdown_keymap(jump_to_bootloader)) {
        return false;
    }

    // functions registered with PEKE_DEINIT
    for (
        deinit_fn *func = &__elpekenin_deinit_base__;
        func < &__elpekenin_deinit_end__;
        ++func
    ) {
        (*func)(jump_to_bootloader);
    }

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
        suspend_debug_state = debug_config.enable;
        debug_config.enable = false;
    }

    // only run suspend-specific code once per suspend duration
    suspend_changed = false;
    suspend_power_down_keymap();
}

void suspend_wakeup_init_user(void) {
    // enable debug after USB is init'ed on startup
    if (!keyboard_booted) {
        keyboard_booted     = true;
        debug_config.enable = true;
        return;
    }

    // flag the suspend callback to handle code on its next trigger
    suspend_changed = true;

    // restore debug and log event after suspend
    debug_config.enable = suspend_debug_state;
    logging(UNKNOWN, LOG_TRACE, "waking up...");

    suspend_wakeup_init_keymap();
}
