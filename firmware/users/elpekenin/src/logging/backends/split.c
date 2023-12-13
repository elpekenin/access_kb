// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>

#include "keyboard.h"
#include "printf.h"
#include "timer.h"

#include "elpekenin/logging.h"
#include "elpekenin/split/transactions.h"
#include "elpekenin/utils/compiler.h"

// *** Split data ***

typedef struct PACKED {
    bool    flush;
    uint8_t bytes;
    uint8_t buff[RPC_S2M_BUFFER_SIZE - 2];
} split_logging_t;
_Static_assert(sizeof(split_logging_t) == RPC_S2M_BUFFER_SIZE, "Wrong size");


// *** Slave *** //

static uint8_t slave_i = 0;
static uint8_t slave_buffer[200] = {0};

void sendchar_split_hook(uint8_t c) {
    // on master, this does nothing
    if (is_keyboard_master()) {
        return;
    }

    // append data
    slave_buffer[slave_i++] = c;

    // we filled the buffer before master asked for data, oops
    // in this unlikely scenario, reset the buffer and log an eror message
    if (slave_i == (ARRAY_SIZE(slave_buffer) - 1)) {
        slave_i = 0;
        memset(slave_buffer, 0, ARRAY_SIZE(slave_buffer));
        logging(LOGGER, LOG_ERROR, "Slave buffer filled");
    }
}

void user_logging_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    // how many bytes we can read
    split_logging_t data = {
        .flush = false,
        .bytes = MIN(slave_i, RPC_S2M_BUFFER_SIZE),
    };

    if (data.bytes == 0) {
        memcpy(s2m_buffer, &data, sizeof(split_logging_t));
        return;
    }

    // copy bytes
    memcpy(data.buff, slave_buffer, data.bytes);

    // move sendchar's buffer content to the beggining
    memmove(slave_buffer, slave_buffer + data.bytes, ARRAY_SIZE(slave_buffer) - data.bytes - 1);
    slave_i -= data.bytes;

    // flush?
    for (uint8_t j = 0; j < data.bytes; ++j) {
        if (data.buff[j] == '\n') {
            data.flush = true;
        }
    }

    memcpy(s2m_buffer, &data, sizeof(split_logging_t));
}


// *** Master ***

static uint8_t master_i = 0;
static uint8_t master_buffer[200] = {0};

static inline void clear_master_buffer(void) {
    master_i = 0;
    memset(master_buffer, 0, ARRAY_SIZE(master_buffer));
}

void user_logging_master_poll(void) {
    split_logging_t data;
    transaction_rpc_recv(RPC_ID_USER_LOGGING, RPC_S2M_BUFFER_SIZE, &data);

    if (data.bytes == 0) {
        return;
    }

    // check size
    if (master_i + data.bytes >= ARRAY_SIZE(master_buffer) - 1) {
        clear_master_buffer();
        logging(LOGGER, LOG_ERROR, "Master buffer filled");
        return;
    }

    // copy received
    memcpy(master_buffer + master_i, data.buff, data.bytes);
    master_i += data.bytes;

    // flush if asked to
    if (data.flush) {
        printf("--- SLAVE ---\n");
        // for now, let's assume flush => '\n' on last position
        printf("%s", master_buffer);
        printf("-------------\n");
        clear_master_buffer();
    }
}