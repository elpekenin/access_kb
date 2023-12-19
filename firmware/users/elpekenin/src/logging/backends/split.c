// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>

#include "keyboard.h"
#include "print.h"

#include "elpekenin/logging.h"
#include "elpekenin/split/transactions.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/string.h"

// *** Split data ***

typedef struct PACKED {
    bool    flush;
    uint8_t bytes;
    char    buff[RPC_S2M_BUFFER_SIZE - 3];
    char    null;
} split_logging_t;
_Static_assert(sizeof(split_logging_t) == RPC_S2M_BUFFER_SIZE, "Wrong size");


// *** Slave ***

static size_t slave_i           = 0;
static char   slave_buffer[200] = {0};

int8_t sendchar_split_hook(uint8_t c) {
    // on master, this does nothing
    if (is_keyboard_master()) {
        return 0;
    }

    if (
        c < '\n' // terminator/non-printable chars
        || is_utf8(c) // only accept ASCII
    ) { 
        return 0;
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

    return 0;
}

void user_logging_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    split_logging_t data = {0};

    uint8_t i;
    size_t max_size = MIN(slave_i, ARRAY_SIZE(data.buff));

    // work out the data
    for (i = 0; i < max_size; ++i) {
        if (!slave_buffer[i]) { // end of buffer contents
            break;
        }

        // copy byte to msg
        data.buff[i] = slave_buffer[i];

        // move char into just-copied position
        slave_buffer[i] = slave_buffer[i + data.bytes];

        // clear the char we just moved
        slave_buffer[i + data.bytes] = '\0';

        // update message: do we have to flush?
        if (data.buff[i] == '\n') {
            data.flush = true;
        }
    }

    // update msg counter
    data.bytes = i;

    // update buffer's write posiiton
    slave_i -= data.bytes;

    // copy data on send buffer
    memcpy(s2m_buffer, &data, sizeof(split_logging_t));
}


// *** Master ***

static size_t master_i           = 0;
static char   master_buffer[200] = {0};

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
        // for now, lets assume flush => '\n' on last position
        printf("%s", master_buffer);
        printf("-------------\n");
        clear_master_buffer();
    }
}
