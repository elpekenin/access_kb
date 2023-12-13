// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include QMK_KEYBOARD_H

#include "elpekenin/layers.h"

#ifndef KEYLOG_SIZE
#    define KEYLOG_SIZE 40
#endif
// this is setup with an extra byte to try and ensure we always have a '\0'
extern char keylog[KEYLOG_SIZE + 1];

extern bool qp_log_redraw;

void keylog_process(uint16_t keycode, keyrecord_t *record);

typedef struct {
    char *find;
    char *replace;
} str_replacement_t;
#define REPLACE(f, r) (str_replacement_t){.find = f, .replace = r}


typedef struct {
    str_replacement_t replace;
    uint8_t           mod_mask;
} mod_replacement_t;
#define MOD_REPLACE(f, r, m) (mod_replacement_t){.replace = REPLACE(f, r), .mod_mask = m}

#define REPLACE_ALGR(f, r) MOD_REPLACE(f, r, MOD_BIT(KC_ALGR))
#define REPLACE_SFT(f, r)  MOD_REPLACE(f, r, MOD_MASK_SHIFT)

void keycode_repr(char **str);
