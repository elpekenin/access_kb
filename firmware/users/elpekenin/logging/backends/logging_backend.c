// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "sendchar.h"

#if defined(QUANTUM_PAINTER_ENABLE)
#    include "qp_logging_backend.h"
#endif

#if defined(SPLIT_KEYBOARD)
#    include "split_logging_backend.h"
#endif

#if defined(XAP_ENABLE)
#    include "xap_logging_backend.h"
#endif

int8_t user_sendchar(uint8_t c) {
    // with debug, each char written would require a QP redraw, meaning more chars being drawn and so on
#if defined(QUANTUM_PAINTER_ENABLE) && !defined(QUANTUM_PAINTER_DEBUG)
    sendchar_qp_hook(c);
#endif

#if defined(SPLIT_KEYBOARD)
    sendchar_split_hook(c);
#endif

#if defined(XAP_ENABLE)
    sendchar_xap_hook(c);
#endif

    // default logging provided by QMK
    //    - USB via console endpoint, if CONSOLE_ENABLE
    //    - no-op otherwise
    return sendchar(c);
}
