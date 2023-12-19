// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin.h"
#include "elpekenin/logging.h"
#include "elpekenin/placeholders.h"

#include "generated/features.h"

// Conditional imports
#if defined(AUTOCORRECT_ENABLE)
#    include "process_autocorrect.h"
#endif

#if defined(GAME_ENABLE)
#    include "elpekenin/game.h"
#endif

#if defined(KEYLOG_ENABLE)
#    include "elpekenin/keylog.h"
#endif

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "elpekenin/qp/graphics.h"
#endif

#if defined(SPLIT_KEYBOARD)
#    include "elpekenin/split/transactions.h"
#endif


void housekeeping_task_user(void) {
    housekeeping_task_keymap();
}

void keyboard_pre_init_user(void) {
    print_set_sendchar(user_sendchar);

    keyboard_pre_init_keymap();
}

void keyboard_post_init_user(void) {
#if defined(AUTOCORRECT_ENABLE)
    autocorrect_enable();
#endif

#if defined(GAME_ENABLE)
    game_init();
#endif

#if defined(QUANTUM_PAINTER_ENABLE)
    elpekenin_qp_init();
#endif

#if defined(SPLIT_KEYBOARD)
    split_init();
#endif

#if defined(TRI_LAYER_ENABLE)
    configure_tri_layer();
#endif

    keyboard_post_init_keymap();
}
