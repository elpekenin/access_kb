// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// re-export QMK utils
#include "quantum/util.h"

#if !defined(PACKED)
#    define PACKED __attribute__((packed))
#endif

#if !defined(UNUSED)
#    define UNUSED __attribute__((unused))
#endif

#if !defined(WEAK)
#    define WEAK __attribute__((weak))
#endif