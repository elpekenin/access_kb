// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "quantum.h"
#include "sendstring_spanish.h"
#define DELAY 10 // ms between sendstring actions

#include "elpekenin/keycodes.h"
#include "elpekenin/logging.h"
#include "elpekenin/placeholders.h"

#if defined(GAME_ENABLE)
#    include "elpekenin/game.h"
#endif

#if defined(KEYLOG_ENABLE)
#    include "elpekenin/keylog.h"
#endif

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "elpekenin/logging/backends/qp.h"
#endif

#if defined(SPLIT_KEYBOARD)
#    include "elpekenin/split/transactions.h"
#endif

#if defined(XAP_ENABLE)
#    include "elpekenin/xap.h"
#endif


// *** Logic start ***

#if defined(AUTOCORRECT_ENABLE)
bool apply_autocorrect(uint8_t backspaces, const char *str, char *typo, char *correct) {
    logging(UNKNOWN, LOG_WARN, "'%s' - '%s'", typo, correct);
    return true;
}

bool process_autocorrect_user(uint16_t *keycode, keyrecord_t *record, uint8_t *typo_buffer_size, uint8_t *mods) {
    switch (*keycode) {
        // TODO: Revisit this. works like crap
        // case TD_SPC:
        //     *keycode              = KC_SPC; // make this look like a regular spacebar
        //     record->event.pressed = false;  // trigger an extra backspace when corrected
        //     return true;

        default:
            return process_autocorrect_default_handler(keycode, record, typo_buffer_size, mods);
    }
}
#endif // defined(AUTOCORRECT_ENABLE)

#if defined(KEYLOG_ENABLE)
bool keylog_enabled = true;
#endif // defined(KEYLOG_ENABLE)

#if defined(KEY_OVERRIDE_ENABLE)
const key_override_t delete_key_override = ko_make_basic(MOD_MASK_SHIFT, KC_BSPC, KC_DEL);
const key_override_t volume_key_override = ko_make_basic(MOD_MASK_SHIFT, KC_VOLU, KC_VOLD);
const key_override_t alt_f4_key_override = ko_make_basic(MOD_MASK_ALT,   KC_4,    A(KC_F4));
const key_override_t **key_overrides = (const key_override_t *[]){
    &delete_key_override,
    &volume_key_override,
    &alt_f4_key_override,
    NULL
};
#endif // defined(KEY_OVERRIDE_ENABLE)


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
#if defined(KEYLOG_ENABLE)
    if (keylog_enabled) {
        keylog_process(keycode, record);
    }
#endif // defined(KEYLOG_ENABLE)

#if defined(XAP_ENABLE)
        xap_keyevent(keycode, record);
#endif // defined(XAP_ENABLE)


    if (!process_record_keymap(keycode, record)) {
        return false;
    }

    bool    pressed = record->event.pressed;
    uint8_t mods    = get_mods();
    bool    l_sft   = mods & MOD_BIT(KC_LSFT);

    switch (keycode) {
#if defined(SPLIT_KEYBOARD)
        case EE_CLR:
            if (pressed) {
                reset_ee_slave(); // reset on slave too
            }
            return true;
#endif // defined(SPLIT_KEYBOARD)

        case PK_CPYR:
            // avoid messing up when i press GUI instead of TRI_LAYER for QK_RST
            if (get_mods() & MOD_MASK_GUI) {
                return false;
            }

            if (pressed) {
                send_string(
                    "// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>\n"
                    "// SPDX-License-Identifier: GPL-2.0-or-later\n"
                );
            }
            return false;

#if defined(QUANTUM_PAINTER_ENABLE)
        case PK_QCLR:
            if (pressed) {
                for (uint8_t i = 0; i < LOG_N_LINES; ++i) {
                    sendchar_qp_hook('\n');
                }
            }
            return false;
#endif // defined(QUANTUM_PAINTER_ENABLE)

#if defined(KEYLOG_ENABLE)
        case PK_KLOG:
            if (pressed) {
                keylog_enabled = !keylog_enabled;
            }
            return false;
    #endif // defined(KEYLOG_ENABLE)

#if defined(UCIS_ENABLE)
        case PK_UCIS:
            if (pressed) {
                ucis_start();
            }
            return false;
#endif // defined(UCIS_ENABLE)

        case PK_LOG:
            if (pressed) {
                step_level_for(UNKNOWN, !l_sft);
            }
            return false;

#if defined(GAME_ENABLE)
        case KC_W:
            set_game_movement(TOP);
            break;

        case KC_A:
            set_game_movement(LEFT);
            break;

        case KC_S:
            set_game_movement(BOTTOM);
            break;

        case KC_D:
            set_game_movement(RIGHT);
            break;

        case PK_GAME:
            if (pressed) {
                game_reset();
            }
            return false;
#endif // defined(GAME_ENABLE)

        default:
            break;
    }

    return true;
}

#if defined(TAP_DANCE_ENABLE)
void td_ntil_finished(tap_dance_state_t *state, void *build_info) {
    switch (state->count) {
        case 1:
            tap_code16(ES_ACUT); // ´
            break;

        case 2:
            tap_code16(ES_NTIL); // ñ
            break;

        default:
            break;
    }
}

void td_z_finished(tap_dance_state_t *state, void *build_info) {
    switch (state->count) {
        case 1:
            register_code16(KC_Z); // z
            break;

        case 2:
            tap_code16(ES_LABK); // <
            break;

        case 3:
            tap_code16(ES_RABK); // >
            break;

        default:
            break;
    }
}

void td_z_reset(tap_dance_state_t *state, void *build_info) {
    switch (state->count) {
        case 1:
            unregister_code16(KC_Z);
    }
}

void td_spc_each(tap_dance_state_t *state, void *build_info) {
    unregister_code16(KC_SPC);
    register_code16(KC_SPC);

    if ((state->count % 2) == 0) {
        unregister_code16(KC_SPC);
        tap_code16(KC_SPC);
        register_code16(KC_SPC);
    }
}

void td_spc_reset(tap_dance_state_t *state, void *build_info) {
    unregister_code16(KC_SPC);
}

void td_grv_finished(tap_dance_state_t *state, void *build_info) {
    if (state->count == 1) {
        tap_code16(ES_GRV);
        return;
    }

    // TODO: Change tap-hold decision so this doesnt work funny
    send_string_with_delay("``` " SS_LSFT("\n\n") "``` ", DELAY);
    tap_code16(KC_UP);
}

tap_dance_action_t tap_dance_actions[] = {
    [_TD_NTIL] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_ntil_finished, NULL),
    [_TD_Z]    = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_z_finished, td_z_reset),
    [_TD_SPC]  = ACTION_TAP_DANCE_FN_ADVANCED(td_spc_each, NULL, td_spc_reset),
    [_TD_GRV]  = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_grv_finished, NULL),
};
#endif // defined(TAP_DANCE_ENABLE)
