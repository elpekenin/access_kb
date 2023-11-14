// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "placeholders.h"
#include "user_utils.h"

WEAK void housekeeping_task_keymap(void) { }

WEAK void keyboard_pre_init_keymap(void) { }

WEAK void keyboard_post_init_keymap(void) { }

WEAK layer_state_t layer_state_set_keymap(layer_state_t state) { return state; }

WEAK bool led_update_keymap(led_t led_state) { return true; }

WEAK bool process_record_keymap(uint16_t keycode, keyrecord_t *record) { return true; }

WEAK bool shutdown_keymap(bool jump_to_bootloader) { return true; }

WEAK void suspend_power_down_keymap(void) { }

WEAK void suspend_wakeup_init_keymap(void) { }
