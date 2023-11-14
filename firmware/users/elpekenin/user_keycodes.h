// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "keycodes.h"
#include "keymap_spanish.h"

#define _______ KC_TRNS
#define R_SPC   LT(_FN3, KC_SPC)

// note: these are always defined, but may not do anything based on features enabled
enum userspace_keycodes {
    __PLACEHOLDER = QK_USER,

    PK_CPYR, // copyright header

    // QP
    PK_QCLR, // clear logging (print)
    PK_KLOG, // toggle keylogger

    // Unicode
    PK_UCIS, // starts UCIS mode

    // First value that can be used on a keymap, keep last
    QK_KEYMAP
};

#if defined(TAP_DANCE_ENABLE)
enum userspace_tapdances {
    _TD_NTIL,
    _TD_Z,
    _TD_GRV,
    _TD_SPC,
};

#    define TD_NTIL TD(_TD_NTIL)
#    define TD_Z    TD(_TD_Z)
#    define TD_GRV  TD(_TD_GRV)
#    define TD_SPC  TD(_TD_SPC)
#else
#    define TD_NTIL ES_NTIL
#    define TD_Z    KC_Z
#    define TD_GRV  ES_GRV
#    define TD_SPC  KC_SPC
#endif // TAP_DANCE_ENABLE
