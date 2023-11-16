// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "transactions.h"

#include "user_data.h"
#include "user_utils.h"
#include "user_logging.h"
#include "user_transactions.h"

// Callbacks
WEAK void user_data_sync_keymap_callback(void) { }

void user_data_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size == sizeof(user_data_t)) {
        memcpy(&user_data, m2s_buffer, sizeof(user_data_t));

        user_data_sync_keymap_callback();
    } else {
        logging(SPLIT, ERROR, "%s", __func__);
    }
}

void user_shutdown_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size == sizeof(bool)) {
        void shutdown_quantum(bool jump_to_bootloader);
        shutdown_quantum(*(bool *)m2s_buffer);
    } else {
        logging(SPLIT, ERROR, "%s", __func__);
    }
}

// Update user data every now and then
void housekeeping_split_sync(uint32_t now) {
    static uint32_t sync_timer = 0;
    if (is_keyboard_master() && TIMER_DIFF_32(now, sync_timer) > 30000) {
        sync_timer = now;
        transaction_rpc_send(RPC_ID_USER_DATA, sizeof(user_data_t), &user_data);
    }
}

// Register messages
void transactions_init(void) {
    memset(&user_data, 0, sizeof(user_data_t));
    transaction_register_rpc(RPC_ID_USER_DATA, user_data_slave_callback);
    transaction_register_rpc(RPC_ID_USER_SHUTDOWN, user_shutdown_slave_callback);
}
