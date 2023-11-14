// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "qp.h"

#include "deferred_exec.h"

#if !defined(LOG_N_LINES)
#    define LOG_N_LINES 9
#endif // !defined(LOG_N_LINES)

#if !defined(LOG_N_CHARS)
#    define LOG_N_CHARS 70
#endif // !defined(LOG_N_CHARS)

#undef QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS
#define QUANTUM_PAINTER_CONCURRENT_SCROLLING_TEXTS (LOG_N_LINES + 5)

void sendchar_qp_hook(uint8_t c);

typedef struct {
    painter_device_t device;
    painter_font_handle_t font;
    uint16_t screen_w;
    uint16_t x;
    uint16_t y;
    uint8_t n_chars;
    uint32_t delay;
} qp_logging_render_args_t;
void qp_logging_render(qp_logging_render_args_t args);
