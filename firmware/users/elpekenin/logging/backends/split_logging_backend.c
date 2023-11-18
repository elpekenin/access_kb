// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>

#include "keyboard.h"
#include "timer.h"

#include "split_logging_backend.h"

#include "logging.h"
#include "user_transactions.h"

static uint8_t i = 0;
static uint8_t buffer[200] = {0};

void sendchar_split_hook(uint8_t c) {
    // on master, this does nothing
    if (is_keyboard_master()) {
        return;
    }

    // append data
    buffer[i++] = c;

    // we filled the buffer before master asked for data, oops
    // in this unlikely scenario, reset the buffer and log an eror message
    if (i == (ARRAY_SIZE(buffer) - 1)) {
        i = 0;
        memset(buffer, 0, ARRAY_SIZE(buffer));
        logging(LOGGER, ERROR, "Slave buffer filled");
    }
}

void consume_split_sendchar(void *dest, uint8_t max_bytes) {
    // how many bytes we can read
    split_logging_t data = {
        .flush = false,
        .bytes = MIN(i, max_bytes),
    };

    if (data.bytes == 0) {
        memcpy(dest, &data, sizeof(split_logging_t));
        return;
    }

    // copy bytes
    memcpy(data.buff, buffer, data.bytes);

    // move sendchar's buffer content to the beggining
    memmove(buffer, buffer + data.bytes, ARRAY_SIZE(buffer) - data.bytes - 1);
    i -= data.bytes;

    // flush?
    for (uint8_t j = 0; j < data.bytes; ++j) {
        if (data.buff[j] == '\n') {
            data.flush = true;
        }
    }

    memcpy(dest, &data, sizeof(split_logging_t));
}