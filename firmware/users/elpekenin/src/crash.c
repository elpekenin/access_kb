// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <osal.h>

#include "backtrace.h"

#include "elpekenin/logging.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/sections.h"


#define MAGIC_VALUE (0xDEADA55)
#define DEPTH (100) /* max size */

typedef struct {
    uint32_t    magic;
    uint8_t     stack_depth;
    backtrace_t call_stack[DEPTH];
    const char *cause;
    char        msg[100];
} crash_info_t;

SECTION(".no_init") static crash_info_t crash_info;
static crash_info_t copied_crash_info = {0};


// *** API ***

static void copy_crash_info(void) {
    memcpy(&copied_crash_info, &crash_info, sizeof(crash_info_t));
}
PEKE_PRE_INIT(copy_crash_info, INIT_CRASH);

bool program_crashed(void) {
    return copied_crash_info.magic == MAGIC_VALUE;
}

const char *get_crash_msg(void) {
    return copied_crash_info.msg;
}

void set_crash_msg(const char *msg) {
    strcpy(crash_info.msg, msg);
}

backtrace_t *get_crash_call_stack(uint8_t *depth) {
    if (!program_crashed()) {
        *depth = 0;
        return NULL;
    }

    *depth = copied_crash_info.stack_depth;
    return copied_crash_info.call_stack;
}

void print_crash_call_stack(void) {
    uint8_t      depth;
    backtrace_t *call_stack = get_crash_call_stack(&depth);

    // first entry is the error handler, skip it
    logging(UNKNOWN, LOG_WARN, "Crash (%s)", copied_crash_info.cause);
    for (uint8_t i = 1; i < depth; ++i) {
        logging(UNKNOWN, LOG_ERROR, "%s", call_stack[i].name);
        logging(UNKNOWN, LOG_ERROR, "%p", call_stack[i].address);
    }
}

void clear_crash_info(void) {
    // make sure we store it before wiping
    memset(&crash_info, 0, sizeof(crash_info));
}


// *** IRQ Handlers ***

#define HANDLER(__func, __name) \
    INTERRUPT NORETURN void __func(void) { \
        crash_info.magic = MAGIC_VALUE; \
        crash_info.stack_depth = backtrace_unwind(crash_info.call_stack, DEPTH); \
        crash_info.cause = __name; \
        NVIC_SystemReset(); \
    }

HANDLER(_unhandled_exception, "Unknown")
HANDLER(HardFault_Handler,    "Hard")
HANDLER(BusFault_Handler,     "Bus")
HANDLER(UsageFault_Handler,   "Usage")
HANDLER(MemManage_Handler,    "MemMan")

// defined by ChibiOS, for context swap (?)
// HANDLER(NMI_Handler);
