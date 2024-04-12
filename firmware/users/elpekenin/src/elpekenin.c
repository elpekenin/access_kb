// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin.h"
#include "elpekenin/crash.h"
#include "elpekenin/logging.h"
#include "elpekenin/placeholders.h"
#include "elpekenin/utils/allocator.h"
#include "elpekenin/utils/sections.h"
#include "elpekenin/utils/string.h"

#include "generated/features.h"

void housekeeping_task_user(void) {
    housekeeping_task_keymap();
}

void keyboard_pre_init_user(void) {
    // functions registered with PEKE_INIT
    FOREACH_SECTION(init_fn, init, func) {
        (*func)();
    }

    keyboard_pre_init_keymap();
}

void keyboard_post_init_user(void) {
    keyboard_post_init_keymap();

    if (program_crashed()) {
        print_crash_call_stack();
        const char *msg = get_crash_msg();
        if (*msg != '\0') {
            logging(UNKNOWN, LOG_ERROR, "Crashed with: %s", msg);
        }
        clear_crash_info();
    }
}

