// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <ch.h>

#include <platforms/gpio.h>
#include <platforms/chibios/gpio.h>

#include "elpekenin/logging.h"
#include "elpekenin/utils/sections.h"

// will be a no-op if PICO_SDK_WRAPPERS is disabled on elpekenin/mk/mcu.mk
void pico_sdk_init(void) {
    // these are defined by PicoSDK
    extern init_fn __preinit_array_base__, __preinit_array_end__;

    for (
        init_fn *func = &__preinit_array_base__;
        func < &__preinit_array_end__;
        func++
    ) {
        (*func)();
    }
}
PEKE_INIT(pico_sdk_init, INIT_SDK);


void c1_main(void) {
    chSysWaitSystemState(ch_sys_running);
    chInstanceObjectInit(&ch1, &ch_core1_cfg);

    chSysUnlock();

    chThdSleepMilliseconds(1000);
    logging(UNKNOWN, LOG_INFO, "Hello from core %d", port_get_core_id());

    while (true) { }
}
