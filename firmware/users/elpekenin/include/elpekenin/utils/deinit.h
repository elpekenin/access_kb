// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/compiler.h"

typedef void (*deinit_fn)(bool);

#define PEKE_DEINIT(func, prio) \
    SECTION(".elpekenin_deinit." #prio "_" #func) USED static deinit_fn ___peke_deinit_##func = func
