// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "deferred_exec.h"
#include "eeconfig.h"

#include "elpekenin/build_info.h"
#include "elpekenin/logging.h"
#include "elpekenin/logging/backends/split.h"
#include "elpekenin/split/transactions.h"
#include "elpekenin/utils/compiler.h"


// *** Callbacks ***

WEAK void build_info_sync_keymap_callback(void) { }

void build_info_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size == sizeof(build_info_t)) {
        build_info_t *build_info = (build_info_t *)m2s_buffer;

        set_build_commit(build_info->commit);
        set_build_features(build_info->features);

        build_info_sync_keymap_callback();
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


// *** Periodic tasks ***

static inline uint32_t build_info_sync_callback(uint32_t trigger_time, void *cb_arg) {
    build_info_t build_info = get_build_info();
    transaction_rpc_send(RPC_ID_BUILD_INFO, sizeof(build_info_t), &build_info);
    return 30000;
}

static inline uint32_t slave_log_sync_callback(uint32_t trigger_time, void *cb_arg) {
    user_logging_master_poll();
    return 3000;
}


// *** Exposed to other places ***

void reset_ee_slave(void) {
    transaction_rpc_send(RPC_ID_USER_EE_CLR, 0, NULL);
}


// *** Register messages ***

void split_init(void) {
    transaction_register_rpc(RPC_ID_BUILD_INFO, build_info_slave_callback);
    transaction_register_rpc(RPC_ID_USER_SHUTDOWN, user_shutdown_slave_callback);
    transaction_register_rpc(RPC_ID_USER_LOGGING, user_logging_slave_callback);
    transaction_register_rpc(RPC_ID_USER_EE_CLR, user_ee_clr_callback);

    if (is_keyboard_master()) {
        // 5 secs to prevent drawing on eInk right after flash
        defer_exec(5000, build_info_sync_callback, NULL);
        defer_exec(10, slave_log_sync_callback,  NULL);
    }
}
