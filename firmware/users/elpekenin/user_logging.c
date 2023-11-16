// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdarg.h>

#include "printf.h"
#include "sendchar.h"

#include "user_logging.h"
#include "user_utils.h"

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "qp_logging.h"
#endif // defined(QUANTUM_PAINTER_ENABLE)

#if defined(XAP_ENABLE)
#    include "xap.h"
#    include "usb_descriptor.h"
#    include "wait.h"
#    define MAX_PAYLOAD_SIZE (XAP_EPSIZE - sizeof(xap_broadcast_header_t))
#endif // defined(XAP_ENABLE)

#if defined(XAP_ENABLE)
void sendchar_xap_hook(uint8_t c) {
    // buffer to store several chars before transmiting
    static uint8_t data[MAX_PAYLOAD_SIZE] = {0};

    // sending early on leads to issues, instead wait a bit for USB to settle
    if (timer_read() < 3000) {
        return;
    }

    // where to store next char
    static uint8_t payload_len = 0;

    // append data
    data[payload_len++] = c;

    // send on '\n' or when buffer is filled
    // -1 because we need to keep last element 0 for terminator
    if (payload_len == (MAX_PAYLOAD_SIZE - 1) || c == '\n') {
        xap_broadcast(0x00, data, payload_len);

        payload_len = 0;
        // small optimization, last one will always stay at 0, dont set
        memset(data, 0, MAX_PAYLOAD_SIZE - 1);
    }
}
#endif // defined(XAP_ENABLE)

int8_t user_sendchar(uint8_t c) {
    // with debug, each char written would require a QP redraw, meaning more chars being drawn and so on
#if defined(QUANTUM_PAINTER_ENABLE) && !defined(QUANTUM_PAINTER_DEBUG)
    sendchar_qp_hook(c);
#endif // defined(QUANTUM_PAINTER_ENABLE) && !defined(QUANTUM_PAINTER_DEBUG)

#if defined(XAP_ENABLE)
    sendchar_xap_hook(c);
#endif // defined(XAP_ENABLE)

    // default logging (USB via console endpoint)
    return sendchar(c);
}


// *** Actual logging ***

// stringify log levels
static const char *level_str[] = {
    [NONE]  = "UNREACHABLE",
    [TRACE] = "TRACE",
    [DEBUG] = "DEBUG",
    [INFO]  = "INFO",
    [WARN]  = "WARN",
    [ERROR] = "ERROR",
};
ASSERT_LEVELS(level_str);

// stringify features
static const char *feature_str[] = {
    [UNKNOWN]    = "",
    [LOGGER]     = "LOG",
    [QP]         = "QP",
    [SCROLL_TXT] = "SCROLL",
    [SIPO]       = "SIPO",
    [SPLIT]      = "SPLIT",
    [SPI]        = "SPI",
    [TOUCH]      = "TOUCH",
};
ASSERT_FEATURES(feature_str);

// logging level for each feature
log_level_t feature_levels[] = {
    [UNKNOWN]    = INFO,
    [LOGGER]     = ERROR,
    [QP]         = ERROR,
    [SCROLL_TXT] = ERROR,
    [SIPO]       = NONE,
    [SPLIT]      = ERROR,
    [SPI]        = NONE,
    [TOUCH]      = ERROR,
};
ASSERT_FEATURES(feature_levels);

log_level_t get_level_for(feature_t feature) {
    return feature_levels[feature];
}

void set_level_for(feature_t feature, log_level_t level) {
    if (
        (feature < UNKNOWN) // is this possible ?
        || (level < NONE)
        || (feature >= __N_FEATURES__)
        || (level >= __N_LEVELS__)
    ) {
        logging(LOGGER, ERROR, "%s", __func__);
        return;
    }

    feature_levels[feature] = level;
}

void step_level_for(feature_t feature, bool increase) {
    log_level_t level = get_level_for(feature);

    if (
        ((level == NONE) && !increase)
        || (((level + 1) == __N_LEVELS__) && increase)
    ) {
        logging(LOGGER, ERROR, "%s", __func__);
        return;
    }

    if (increase) {
        level++;
    } else {
        level--;
    }

    set_level_for(feature, level);
}

// internals
static token_t get_token(const char **str) {
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

    logging(LOGGER, ERROR, "%s", __func__);
    return INVALID_SPEC;
}

// format used in the logger
static const char *fmt = "[%LS] (%F) %M\n";

bool set_logging_fmt(const char *new_fmt) {
    const char *copy = new_fmt;
    while (1) {
        token_t spec = get_token(&copy);

        if (spec == STR_END) {
            fmt = new_fmt;
            return true;
        }

        if (spec == INVALID_SPEC) {
            logging(LOGGER, ERROR, "Invalid fmt");
            return false;
        }

        copy++;
    }
}

static log_level_t msg_level = NONE; // level of the text being logged

log_level_t get_message_level(void) {
    return msg_level;
}

WEAK char *log_time(void) {
    static char buff[10] = {0};
    _itoa(timer_read32() / 1000, buff);
    return buff;
}

void logging(feature_t feature, log_level_t level, const char *msg, ...) {
    // message filtered out, quit
    log_level_t feat_level = feature_levels[feature];
    if (level < feat_level || feat_level == NONE) {
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
                logging(LOGGER, ERROR, "???");
                return;

            case NO_SPEC: // print any char
                putchar_(*copy);
                break;

            case STR_END:
                msg_level = NONE;
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
