// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/compiler.h"

typedef void (*init_fn)(void);

#ifndef PEKE_INIT
#    define PEKE_INIT(func, prio) \
        SECTION(".elpekenin_init." #prio "_" #func) USED static init_fn ___peke_init_##func = func
#endif