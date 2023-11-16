// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "version.h"

#include "elpekenin.h"
#include "user_data.h"
#include "user_logging.h"
#include "placeholders.h"

// My generated files
#include "generated_features.h"

// Conditional imports
#if defined(QUANTUM_PAINTER_ENABLE)
#    include "graphics.h"
#endif // defined(QUANTUM_PAINTER_ENABLE)

#if defined(SPLIT_KEYBOARD)
#    include "user_transactions.h"
#endif // defined(SPLIT_KEYBOARD)

#if defined(KEYLOG_ENABLE)
#    include "user_keylog.h"
#endif // defined (KEYLOG_ENABLE)


void housekeeping_task_user(void) {
    __attribute__((unused)) uint32_t now  = timer_read32();

#if defined(QUANTUM_PAINTER_ENABLE)
    housekeeping_qp(now);
#endif // defined(QUANTUM_PAINTER_ENABLE)

#if defined(SPLIT_KEYBOARD)
    housekeeping_split_sync(now);
#endif // defined(SPLIT_KEYBOARD)

#if defined(DEBUG_MATRIX_SCAN_RATE)
    static uint32_t last_matrix = 0;
    if (TIMER_DIFF_32(now, last_matrix) > 6000) {
        last_matrix = now;
        logging(UNKNOWN, DEBUG, "Scans: %ld", get_matrix_scan_rate());
    }
#endif // defined(DEBUG_MATRIX_SCAN_RATE)

    housekeeping_task_keymap();
}

void keyboard_pre_init_user(void) {
    print_set_sendchar(user_sendchar);

    keyboard_pre_init_keymap();
}

user_data_t user_data = {0};
void keyboard_post_init_user(void) {
#if defined(AUTOCORRECT_ENABLE)
    autocorrect_enable();
#endif // defined(AUTOCORRECT_ENABLE)

#if defined(QUANTUM_PAINTER_ENABLE)
    load_qp_resources();
#endif // defined(QUANTUM_PAINTER_ENABLE)

#if defined(SPLIT_KEYBOARD)
    transactions_init();
    // has to be after transactions_init, because it memset's user_data to 0
    if (is_keyboard_master()) {
#endif // defined(SPLIT_KEYBOARD)

        user_data = (user_data_t) {
            .commit   = QMK_GIT_HASH,
            .features = get_enabled_features(),
        };

#if defined(SPLIT_KEYBOARD)
    }
#endif // defined(SPLIT_KEYBOARD)

#if defined(TRI_LAYER_ENABLE)
    configure_tri_layer();
#endif // defined(TRI_LAYER_ENABLE)

    keyboard_post_init_keymap();
}
