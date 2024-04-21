// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <ch.h>

#include "elpekenin/errno.h"
#include "elpekenin/logging.h"
#include "elpekenin/utils/sections.h"

volatile static bool ready = false;

static void signal_c1(void) {
    ready = true;
}
PEKE_POST_INIT(signal_c1, INIT_DONE);


// will be a no-op if PICO_SDK_WRAPPERS is disabled on elpekenin/mk/mcu.mk
static void pico_sdk_init(void) {
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
PEKE_PRE_INIT(pico_sdk_init, INIT_SDK);


#if defined(SECOND_CORE_TASKS)
extern void __real_qp_internal_task(void);
extern void __real_deferred_exec_task(void);
extern void __real_housekeeping_task(void);

void __wrap_qp_internal_task(void) { }
void __wrap_deferred_exec_task(void) { }
void __wrap_housekeeping_task(void) { }
#endif

void c1_main(void) {
    chSysWaitSystemState(ch_sys_running);
    chInstanceObjectInit(&ch1, &ch_core1_cfg);

    chSysUnlock();

    // wait for everything to be configured
    while (!ready) { }

    logging(UNKNOWN, LOG_INFO, "Hello from core 1");

    while (true) {
#if defined(SECOND_CORE_TASKS)
        __real_qp_internal_task();
        __real_deferred_exec_task();
        __real_housekeeping_task();
#endif
    }
}
