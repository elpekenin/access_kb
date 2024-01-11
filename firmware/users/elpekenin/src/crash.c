// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <ch.h>

#include "elpekenin/logging.h"
#include "elpekenin/3rd_party/backtrace.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/shortcuts.h"


#define MAGIC_VALUE (0xDEADA55)
#define DEPTH (100) /* max size */

typedef struct {
    uint32_t    magic;
    uint8_t     stack_depth;
    backtrace_t call_stack[DEPTH];
    const char *cause;
} crash_info_t;

static crash_info_t crash_info SECTION(".no_init");

// *** API ***

bool program_crashed(void) {
    static bool read = false;
    static bool crashed;

    if (!read) {
        read = true;
        crashed = crash_info.magic == MAGIC_VALUE;
    }

    return crashed;
}

void print_crash(void) {
    logging(UNKNOWN, LOG_WARN, "Crash (%s)", crash_info.cause);

    // first entry is the error handler, skip it
    for (uint8_t i = 1; i < crash_info.stack_depth; ++i) {
        backtrace_t frame = crash_info.call_stack[i];
        logging(UNKNOWN, LOG_ERROR, "%s", frame.name);
    }
}

void clear_crash_info(void) {
    // make sure we store it before wiping
    program_crashed();
    WIPE_VAR(crash_info);
}


/// *** IRQ Handlers ***

#define HANDLER(__name) \
    void __name(void) { \
        crash_info.magic = MAGIC_VALUE; \
        crash_info.stack_depth = backtrace_unwind(crash_info.call_stack, DEPTH); \
        crash_info.cause = #__name; \
        NVIC_SystemReset(); \
    }

HANDLER(_unhandled_exception)
HANDLER(HardFault_Handler)
HANDLER(BusFault_Handler)
// HANDLER(NMI_Handler); // !! defined at lib/chibios/os/common/ports/ARMv6-M-RP2/chcore.c
HANDLER(UsageFault_Handler)
HANDLER(MemManage_Handler)
