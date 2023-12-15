// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "eeconfig.h"
#include "print.h"
#include "version.h"

#include "elpekenin.h"
#include "elpekenin/build_info.h"
#include "elpekenin/logging.h"
#include "elpekenin/placeholders.h"

// Conditional imports
#if defined(AUTOCORRECT_ENABLE)
#    include "process_autocorrect.h"
#endif

#if defined(GAME_ENABLE)
#    include "elpekenin/game.h"
#endif // defined(GAME_ENABLE)

#if defined(KEYLOG_ENABLE)
#    include "elpekenin/keylog.h"
#endif // defined (KEYLOG_ENABLE)

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "elpekenin/qp/graphics.h"
#endif // defined(QUANTUM_PAINTER_ENABLE)

#if defined(SPLIT_KEYBOARD)
#    include "elpekenin/split/transactions.h"
#endif // defined(SPLIT_KEYBOARD)

#include "generated/features.h"


void housekeeping_task_user(void) {
    __attribute__((unused)) uint32_t now  = timer_read32();

#if defined(GAME_ENABLE)
    static uint32_t last_frame = 0;
    if (TIMER_DIFF_32(now, last_frame) > 500) {
        last_frame = now;
        game_frame();
    }
#endif // defined(GAME_ENABLE)

#if defined(QUANTUM_PAINTER_ENABLE)
    scrolling_text_tick();
#endif // defined(QUANTUM_PAINTER_ENABLE)

#if defined(SPLIT_KEYBOARD)
    housekeeping_split_sync(now);
#endif // defined(SPLIT_KEYBOARD)

    housekeeping_task_keymap();
}

void keyboard_pre_init_user(void) {
    print_set_sendchar(user_sendchar);

    keyboard_pre_init_keymap();
}

build_info_t build_info = {0};
void keyboard_post_init_user(void) {
#if defined(AUTOCORRECT_ENABLE)
    autocorrect_enable();
#endif // defined(AUTOCORRECT_ENABLE)

#if defined(QUANTUM_PAINTER_ENABLE)
    elpekenin_qp_init();
#endif // defined(QUANTUM_PAINTER_ENABLE)

#if defined(SPLIT_KEYBOARD)
    transactions_init();
    // has to be after transactions_init, because it memset's build_info to 0
    if (is_keyboard_master()) {
#endif // defined(SPLIT_KEYBOARD)

        build_info = (build_info_t) {
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

#if defined(GAME_ENABLE)
    // TODO: Abstract this shit
    // **after** init_keymap to access the display
    game_init(qp_get_device_by_name("ili9341"));
#endif // defined(GAME_ENABLE)
}
