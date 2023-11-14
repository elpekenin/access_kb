// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdarg.h>

#include "printf.h"
#include "sendchar.h"

#include "user_logging.h"

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

// Log-level printing
log_level_t logging_level = NONE;

// logging only supports single-line strings, otherwise color would be a pain to handle
// dont add any '\n', as it will be inserted automatically
static void __log(log_level_t level, const char *fmt, va_list *args) {
    const char *c = fmt;

    // validation: dont allow newlines (nor %s, which could also contain them)
    while (*c != 0) {
        if (*c == '\n' || (*c == '%' && *(c+1) == 's')) {
            // NOTE for future self
            log_error("<INVALID LOG>");
            return;
        }
        c++;
    }

    // update level, so sendchar hooks can change their behaviour
    logging_level = level;

    // prefix
    switch (level) {
        case TRACE: printf("[TRACE] "); break;
        case DEBUG: printf("[DEBUG] "); break;
        case INFO:  printf("[INFO] ");  break;
        case WARN:  printf("[WARN] ");  break;
        case ERROR: printf("[ERROR] "); break;
        default:                        break;
    }

    // print actual string
    vprintf(fmt, *args);

    // newline after string
    printf("\n");

    // restore to non-log mode
    logging_level = NONE;
}

void log_trace(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    __log(TRACE, fmt, &args);
    va_end(args);
}

void log_debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    __log(DEBUG, fmt, &args);
    va_end(args);
}

void log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    __log(INFO, fmt, &args);
    va_end(args);
}

void log_warn(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    __log(WARN, fmt, &args);
    va_end(args);
}

void log_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    __log(ERROR, fmt, &args);
    va_end(args);
}
