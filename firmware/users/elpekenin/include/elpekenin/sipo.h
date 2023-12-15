// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "elpekenin/utils/shortcuts.h"

#define GET_SIPO_BIT(x, bit) GET_BIT(sipo_pin_state[x], bit)
#define PRINT_SIPO_BYTE(x)       \
        logging(SIPO, LOG_DEBUG, \
            "%d%d%d%d%d%d%d%d",  \
            GET_SIPO_BIT(x, 0),  \
            GET_SIPO_BIT(x, 1),  \
            GET_SIPO_BIT(x, 2),  \
            GET_SIPO_BIT(x, 3),  \
            GET_SIPO_BIT(x, 4),  \
            GET_SIPO_BIT(x, 5),  \
            GET_SIPO_BIT(x, 6),  \
            GET_SIPO_BIT(x, 7)   \
        )
#define PRINT_SIPO_STATUS()                    \
        logging(SIPO, LOG_DEBUG, "MCU");       \
        for (int i=_SIPO_BYTES-1; i>=0; --i) { \
            PRINT_SIPO_BYTE(i);                \
        }                                      \
        logging(SIPO, LOG_DEBUG, "END")

// compute the amount of bytes needed
#define _SIPO_BYTES ((N_SIPO_PINS+7)/8)

// create pin lists
#define configure_sipo_pins(...)           \
        enum { __VA_ARGS__, __SIPO_PINS }; \
        _Static_assert(__SIPO_PINS <= N_SIPO_PINS, "Defined more pin names than the amount configured")

// control the buffer
#define sipo_buffer_high(v) set_sipo_pin(v, true)
#define sipo_buffer_low(v) set_sipo_pin(v, false)
void set_sipo_pin(uint8_t position, bool state);

// control the hardware by updating and flushing the buffer
#define sipo_write_high(v) sipo_buffer_high(v); send_sipo_state()
#define sipo_write_low(v) sipo_buffer_low(v); send_sipo_state()

// set status
void send_sipo_state(void);
