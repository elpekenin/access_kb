// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <stdint.h>

typedef enum {
    NONE,
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    __LAST__
} log_level_t;

// to change printf's "backend", dont use.
int8_t user_sendchar(uint8_t c);

// logging functions
extern log_level_t logging_level;

void log_trace(const char *fmt, ...);
void log_debug(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_error(const char *fmt, ...);
