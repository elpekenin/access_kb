// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

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

int8_t _sendchar(uint8_t c) {
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
