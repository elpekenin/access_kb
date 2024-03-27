// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "elpekenin/utils/init.h"

// these are defined by PicoSDK
extern init_fn __preinit_array_base__,
               __preinit_array_end__;

void pico_sdk_init(void) {
    for (
        init_fn *func = &__preinit_array_base__;
        func < &__preinit_array_end__;
        func++
    ) {
        (*func)();
    }
}
PEKE_INIT(pico_sdk_init, 0);