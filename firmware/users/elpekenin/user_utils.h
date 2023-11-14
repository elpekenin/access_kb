// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

// In no particular order

#pragma once

#define MODIFIERS() (get_mods() | get_weak_mods() | get_oneshot_mods())

#define u16_TO_u8(x) ((x) & 0xFF), ((x) >> 8)

#define MS_IN_A_SEC   (1000)
#define MS_IN_A_MIN   (MS_IN_A_SEC   * 60)
#define MS_IN_AN_HOUR (MS_IN_A_MIN   * 60)
#define MS_IN_A_DAY   (MS_IN_AN_HOUR * 24)

#include "color.h" // defines `PACKED`

#if !defined(PACKED)
#    define PACKED __attribute__((packed))
#endif

#if !defined(UNUSED)
#    define UNUSED __attribute__((unused))
#endif

#if !defined(WEAK)
#    define WEAK __attribute__((weak))
#endif
