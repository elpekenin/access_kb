// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "user_utils.h"
#include "transport.h"

typedef struct {
    bool    flush;
    uint8_t bytes;
    uint8_t buff[RPC_S2M_BUFFER_SIZE - 2];
} PACKED split_logging_t;
_Static_assert(sizeof(split_logging_t) == RPC_S2M_BUFFER_SIZE, "Wrong size");

void sendchar_split_hook(uint8_t c);
void consume_split_sendchar(void *dest, uint8_t max_bytes);