// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>

#include "printf.h"
#include "transactions.h"

#include "split_logging_backend.h"

#include "logging.h"
#include "user_data.h"
#include "user_utils.h"
#include "user_transactions.h"

// Callbacks
WEAK void user_data_sync_keymap_callback(void) { }

void user_data_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size == sizeof(user_data_t)) {
        memcpy(&user_data, m2s_buffer, sizeof(user_data_t));

        user_data_sync_keymap_callback();
    } else {
        logging(SPLIT, ERROR, "%s size", __func__);
    }
}

void user_shutdown_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size == sizeof(bool)) {
        void shutdown_quantum(bool jump_to_bootloader);
        shutdown_quantum(*(bool *)m2s_buffer);
    } else {
        logging(SPLIT, ERROR, "%s size", __func__);
    }
}

// *** Logging ***
static uint8_t i = 0;
static uint8_t reception_buffer[200] = {0};

static inline void clear_buffer(void) {
    i = 0;
    memset(reception_buffer, 0, ARRAY_SIZE(reception_buffer));
}

void user_logging_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    consume_split_sendchar(s2m_buffer, RPC_S2M_BUFFER_SIZE);
}

void user_logging_master_receive(void) {
    split_logging_t data;
    transaction_rpc_recv(RPC_ID_USER_LOGGING, RPC_S2M_BUFFER_SIZE, &data);

    if (data.bytes == 0) {
        return;
    }

    // check size
    if (i + data.bytes >= ARRAY_SIZE(reception_buffer) - 1) {
        clear_buffer();
        logging(LOGGER, ERROR, "Master buffer filled");
        return;
    }

    // copy received
    memcpy(reception_buffer + i, data.buff, data.bytes);
    i += data.bytes;

    // flush if asked to
    if (data.flush) {
        printf("--- SLAVE ---\n");
        // for now, let's assume flush => '\n' on last position
        printf("%s", reception_buffer);
        printf("-------------\n");
        clear_buffer();
    }
}
// *** ------- ***

// Update user data, and logging, every now and then
void housekeeping_split_sync(uint32_t now) {
    if (!is_keyboard_master()) {
        return;
    }
    
    static uint32_t data_sync_timer = 0;
    if (TIMER_DIFF_32(now, data_sync_timer) > 30000) {
        data_sync_timer = now;
        transaction_rpc_send(RPC_ID_USER_DATA, sizeof(user_data_t), &user_data);
    }

    static uint32_t log_sync_timer = 0;
    if (TIMER_DIFF_32(now, log_sync_timer) > 3000) {
        log_sync_timer = now;
        user_logging_master_receive();
    } 
}

// Register messages
void transactions_init(void) {
    memset(&user_data, 0, sizeof(user_data_t));
    transaction_register_rpc(RPC_ID_USER_DATA, user_data_slave_callback);
    transaction_register_rpc(RPC_ID_USER_SHUTDOWN, user_shutdown_slave_callback);
    transaction_register_rpc(RPC_ID_USER_LOGGING, user_logging_slave_callback);
}
