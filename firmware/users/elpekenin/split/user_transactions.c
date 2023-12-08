// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>

#include "eeconfig.h"
#include "transactions.h"

#include "split_logging_backend.h"

#include "logging.h"
#include "user_data.h"
#include "user_utils.h"
#include "user_transactions.h"


// *** Callbacks ***

WEAK void user_data_sync_keymap_callback(void) { }

void user_data_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size == sizeof(user_data_t)) {
        memcpy(&user_data, m2s_buffer, sizeof(user_data_t));

        user_data_sync_keymap_callback();
    } else {
        logging(SPLIT, LOG_ERROR, "%s size", __func__);
    }
}

void user_shutdown_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size == sizeof(bool)) {
        void shutdown_quantum(bool jump_to_bootloader);
        shutdown_quantum(*(bool *)m2s_buffer);
    } else {
        logging(SPLIT, LOG_ERROR, "%s size", __func__);
    }
}

void user_ee_clr_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size == 0) {
        eeconfig_init();
    } else {
        logging(SPLIT, LOG_ERROR, "%s size", __func__);
    }
}

void user_ee_test_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size == sizeof(uint32_t)) {
        eeconfig_update_user(*(uint32_t*)m2s_buffer);
    } else {
        logging(SPLIT, LOG_ERROR, "%s size", __func__);
    }
}


// *** Periodic task ***

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
        user_logging_master_poll();
    } 
}


// *** Exposed to other places ***

void reset_ee_slave(void) {
    transaction_rpc_send(RPC_ID_USER_EE_CLR, 0, NULL);
}


// *** Register messages ***

void transactions_init(void) {
    memset(&user_data, 0, sizeof(user_data_t));
    transaction_register_rpc(RPC_ID_USER_DATA, user_data_slave_callback);
    transaction_register_rpc(RPC_ID_USER_SHUTDOWN, user_shutdown_slave_callback);
    transaction_register_rpc(RPC_ID_USER_LOGGING, user_logging_slave_callback);
    transaction_register_rpc(RPC_ID_USER_EE_CLR, user_ee_clr_callback);
}
