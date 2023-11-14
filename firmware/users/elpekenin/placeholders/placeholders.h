// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "action_layer.h"
#include "led.h"

void housekeeping_task_keymap(void);

void keyboard_pre_init_keymap(void);

void keyboard_post_init_keymap(void);

layer_state_t layer_state_set_keymap(layer_state_t state);

bool led_update_keymap(led_t led_state);

bool process_record_keymap(uint16_t keycode, keyrecord_t *record);

bool shutdown_keymap(bool jump_to_bootloader);

void suspend_power_down_keymap(void);

void suspend_wakeup_init_keymap(void);
