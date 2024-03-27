// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin.h"
#include "elpekenin/crash.h"
#include "elpekenin/logging.h"
#include "elpekenin/placeholders.h"
#include "elpekenin/utils/init.h"


#include "generated/features.h"

char g_scratch[100];

void housekeeping_task_user(void) {
    housekeeping_task_keymap();
}

// positions of the first and last entries
// each entry is a pointer to an init func
extern init_fn __elpekenin_init_base__,
               __elpekenin_init_end__;

void keyboard_pre_init_user(void) {
    // functions registered with PEKE_INIT
    for (
        init_fn *func = &__elpekenin_init_base__;
        func < &__elpekenin_init_end__;
        ++func
    ) {
        (*func)();
    }

    print_set_sendchar(user_sendchar);

    keyboard_pre_init_keymap();
}

void keyboard_post_init_user(void) {
    keyboard_post_init_keymap();

    if (program_crashed()) {
        print_crash_call_stack();
        clear_crash_info();
    }
}
