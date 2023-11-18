// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>
#include "user_utils.h"

// to change printf's "backend", dont use.
int8_t user_sendchar(uint8_t c);

// *** Custom logger *** (inspired by python's logging module)
typedef enum {
    UNKNOWN,
    LOGGER,
    QP,
    SCROLL_TXT,
    SIPO,
    SPLIT,
    SPI,
    TOUCH,
    __N_FEATURES__
} feature_t;

typedef enum {
    NONE,
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    __N_LEVELS__
} log_level_t;

log_level_t get_level_for(feature_t feature);
void        set_level_for(feature_t feature, log_level_t level);
void        step_level_for(feature_t feature, bool increase);

void logging(feature_t feature, log_level_t level, const char *msg, ...);

// for sendchar backends to see the current message's level
log_level_t get_message_level(void);

// Change format used to print messages, can contain any text BUT \n
typedef enum {
    STR_END,
    NO_SPEC,
    INVALID_SPEC,
    F_SPEC,
    LL_SPEC,
    LS_SPEC,
    M_SPEC,
    PERC_SPEC,
    T_SPEC,
} token_t;

// TODO?: Per-level or per-(feature+level) formatting

// Specifiers supported:
    // %F - The feature's name (nothing if `UNKNOWN`)

    // %LL - The message's level (long)  | TRACE
    // %LS - The message's level (short) | T

    // %M - The actual message specified by `fmt` and `...` on `logging` - can contain regular specifiers

    // %T - Current time, you can override `char *log_time(void);` func to hook it with a RTC or whatever. Defaults to seconds since boot 
    //    >>> itoa(timer_read32() / 1000)

    // %% - Write a "%"

char *log_time(void);
// returns whether the format was valid (thus, got applied)
bool set_logging_fmt(const char *fmt);

#define ASSERT_FEATURES(__array) _Static_assert(ARRAY_SIZE(__array) == __N_FEATURES__, "Wrong size")
#define ASSERT_LEVELS(__array) _Static_assert(ARRAY_SIZE(__array) == __N_LEVELS__, "Wrong size")