// Copyright 2024 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdarg.h>
#include <string.h>

#include "print.h"
#include "timer.h"

#include "elpekenin/logging.h"
#include "elpekenin/utils/string.h"

// *** Actual logging ***

// stringify log levels
static const char *level_str[] = {
    [LOG_NONE]  = "UNREACHABLE",
    [LOG_TRACE] = "TRACE",
    [LOG_DEBUG] = "DEBUG",
    [LOG_INFO]  = "INFO",
    [LOG_WARN]  = "WARN",
    [LOG_ERROR] = "ERROR",
};
ASSERT_LEVELS(level_str);

// stringify features
static const char *feature_str[] = { // sorted alphabetically
    [UNKNOWN] = "",
    [ALLOC]   = "ALLOC",
    [MAP]     = "MAP",
    [LOGGER]  = "LOG",
    [QP]      = "QP",
    [SCROLL]  = "SCROLL",
    [SIPO]    = "SIPO",
    [SPLIT]   = "SPLIT",
    [SPI]     = "SPI",
    [TOUCH]   = "TOUCH",
};
ASSERT_FEATURES(feature_str);

// logging level for each feature
log_level_t feature_levels[] = { // sorted alphabetically
    [UNKNOWN] = LOG_INFO,
    [ALLOC]   = LOG_ERROR,
    [MAP]     = LOG_ERROR,
    [LOGGER]  = LOG_ERROR,
    [QP]      = LOG_INFO,
    [SCROLL]  = LOG_ERROR,
    [SIPO]    = LOG_NONE,
    [SPLIT]   = LOG_ERROR,
    [SPI]     = LOG_NONE,
    [TOUCH]   = LOG_ERROR,
};
ASSERT_FEATURES(feature_levels);

log_level_t get_level_for(feature_t feature) {
    return feature_levels[feature];
}

static inline void __logging_error(void) {
    logging(LOGGER, LOG_ERROR, "%s", __func__);
}

void set_level_for(feature_t feature, log_level_t level) {
    if (
        (feature < UNKNOWN) // is this possible ?
        || (level < LOG_NONE)
        || (feature >= __N_FEATURES__)
        || (level >= __N_LEVELS__)
    ) {
        return __logging_error();
    }

    feature_levels[feature] = level;
}

void step_level_for(feature_t feature, bool increase) {
    log_level_t level = get_level_for(feature);

    if (
        ((level == LOG_NONE) && !increase)
        || (((level + 1) == __N_LEVELS__) && increase)
    ) {
        return __logging_error();
    }

    if (increase) {
        level++;
    } else {
        level--;
    }

    set_level_for(feature, level);
}

// internals
NON_NULL(1) READ_ONLY(1) static token_t get_token(const char **str) {
    if (**str == '\0') { // null terminator
        return STR_END;
    }

    if (**str != '%') { // no specifier, regular text
        return NO_SPEC;
    }

    (*str)++;
    switch (**str) {
        case 'L':
            (*str)++;
            switch (**str) {
                case 'L': // %LL
                    return LL_SPEC;

                case 'S': // %LS
                    return LS_SPEC;
                
                default:
                    return INVALID_SPEC;
            }

        case 'F': // %F
            return F_SPEC;

        case 'M': // %M
            return M_SPEC;


        case 'T': // %T
            return T_SPEC;

        case '%': // %%
            return PERC_SPEC;

        default:
            return INVALID_SPEC;
    }

    __logging_error();
    return INVALID_SPEC;
}

// format used in the logger
static char fmt[MAX_LOG_FMT_LEN] = "[%LS] (%F) %M\n";

// current size, not max size
uint8_t get_logging_fmt_len(void) {
    return strlen(fmt) + 1;
}

void get_logging_fmt(char *dest) {
    strcpy(dest, fmt);
}

bool set_logging_fmt(const char *new_fmt) {
    if (strlen(new_fmt) >= MAX_LOG_FMT_LEN) {
        logging(LOGGER, LOG_ERROR, "Format too long");
        return false;
    }

    const char *copy = new_fmt;
    while (1) {
        token_t spec = get_token(&copy);

        if (spec == STR_END) {
            strcpy(fmt, new_fmt);
            return true;
        }

        if (spec == INVALID_SPEC) {
            logging(LOGGER, LOG_ERROR, "Invalid format");
            return false;
        }

        copy++;
    }
}

static log_level_t msg_level = LOG_NONE; // level of the text being logged

log_level_t get_message_level(void) {
    return msg_level;
}

WEAK const char *log_time(void) {
    static char buff[10] = {0};
    return _itoa(timer_read32() / 1000, buff);
}

void logging(feature_t feature, log_level_t level, const char *msg, ...) {
    // message filtered out, quit
    log_level_t feat_level = feature_levels[feature];
    if (level < feat_level || feat_level == LOG_NONE) {
        return;
    }

    // set msg lvel
    msg_level = level;

    va_list args;
    const char *copy = fmt;

    // set_format does not allow setting an invalid format, just go thru it
    while (1) {
        // order specs alphabetically, special cases first
        switch (get_token(&copy)) {
            case INVALID_SPEC: // unreachable, guarded by set_logging_fmt
                logging(LOGGER, LOG_ERROR, "???");
                return;

            case NO_SPEC: // print any char
                putchar_(*copy);
                break;

            case STR_END:
                msg_level = LOG_NONE;
                return;

            // ----------

            case F_SPEC: // print feature name
                printf("%s", feature_str[feature]);
                break;

            case LL_SPEC: // print log level (long)
                printf("%s", level_str[level]);
                break;

            case LS_SPEC: // print log level (short)
                putchar_(level_str[level][0]);
                break;

            case M_SPEC: // print actual message
                va_start(args, msg);
                vprintf(msg, args);
                va_end(args);
                break;

            case PERC_SPEC: // print a '%'
                putchar_('%');
                break;

            case T_SPEC: // print current time
                printf("%s", log_time());
                break;
        }

        copy++;
    }
}

void print_str(const char *str, const sendchar_func_t func) {
    for (size_t i = 0; i < strlen(str); ++i) {
        func(str[i]);
    }
}

void print_u8(const uint8_t val, const sendchar_func_t func) {
    // max(u8) == 255 == 4 bytes, extra one for safety
    char txt[5] = {0};
    print_str(_itoa(val, txt), func);
}

void print_u8_array(const uint8_t *list, const size_t len, const sendchar_func_t func) {
    func('[');
    for (size_t i = 0; i < len - 1; ++i) {
        print_u8(list[i], func);
        print_str(", ", func);
    }
    print_u8(list[len - 1], func);
    func(']');
}

void dump_stack(void) {
    backtrace_t call_stack[50];
    uint8_t depth = backtrace_unwind(call_stack, ARRAY_SIZE(call_stack));

    logging(UNKNOWN, LOG_ERROR, "Crash traceback");
    for (
        backtrace_t *stack_frame = call_stack;
        stack_frame < &call_stack[depth];
        ++stack_frame
    ) { // 1 to ignore `dump_stack` itself
        logging(UNKNOWN, LOG_ERROR, "%s@%p", stack_frame->name, stack_frame->function);
    }
}