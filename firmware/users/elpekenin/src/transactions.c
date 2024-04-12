// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <quantum/deferred_exec.h>
#include <quantum/eeconfig.h>

#include "elpekenin.h"
#include "elpekenin/build_info.h"
#include "elpekenin/logging.h"
#include "elpekenin/logging/backends/split.h"
#include "elpekenin/split/transactions.h"
#include "elpekenin/utils/compiler.h"
#include "elpekenin/utils/sections.h"

static inline void __split_size_err(void) {
    logging(SPLIT, LOG_ERROR, "%s size", __func__);
}

// *** Callbacks ***

WEAK void build_info_sync_keymap_callback(void) { }

void build_info_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size != sizeof(build_info_t)) {
        return __split_size_err();
    }

    build_info_t *build_info = (build_info_t *)m2s_buffer;

    set_build_commit(build_info->commit);
    set_build_features(build_info->features);

    build_info_sync_keymap_callback();
}

void user_shutdown_slave_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size != sizeof(bool)) {
        return __split_size_err();
    }

    void shutdown_quantum(bool jump_to_bootloader);
    shutdown_quantum(*(bool *)m2s_buffer);
}

void user_ee_clr_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size != 0) {
        return __split_size_err();
    }

    eeconfig_init();
}


extern void xap_receive_base(const void *data);

void user_xap_callback(uint8_t m2s_size, const void* m2s_buffer, uint8_t s2m_size, void* s2m_buffer) {
    if (m2s_size != XAP_EPSIZE) {
        return __split_size_err();
    }

    xap_receive_base(m2s_buffer);
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

void xap_execute_slave(const void *data) {
    xap_split_msg_t msg = {0};
    // user, qp, operation = 3
    memcpy(&msg, data - sizeof(xap_request_header_t) - 3, XAP_EPSIZE);
    transaction_rpc_send(RPC_ID_XAP, XAP_EPSIZE, &msg);
}

// *** Register messages ***

static void split_init(void) {
    transaction_register_rpc(RPC_ID_BUILD_INFO, build_info_slave_callback);
    transaction_register_rpc(RPC_ID_USER_SHUTDOWN, user_shutdown_slave_callback);
    transaction_register_rpc(RPC_ID_USER_LOGGING, user_logging_slave_callback);
    transaction_register_rpc(RPC_ID_USER_EE_CLR, user_ee_clr_callback);
    transaction_register_rpc(RPC_ID_XAP, user_xap_callback);

    // race condition, QMK fills the left/master fields **after** this func is called
    // lets manually invoke that logic to get it working
    bool is_keyboard_master_impl(void);
    if (is_keyboard_master_impl()) {
        // 5 secs to prevent drawing on eInk right after flash
        defer_exec(5000, build_info_sync_callback, NULL);
        defer_exec(10, slave_log_sync_callback, NULL);
    }
}
PEKE_INIT(split_init, INIT_SPLIT);

static void split_deinit(bool jump_to_bootloader) {
    if (is_keyboard_master()) {
        transaction_rpc_send(RPC_ID_USER_SHUTDOWN, sizeof(jump_to_bootloader), &jump_to_bootloader);
    }
}
PEKE_DEINIT(split_deinit, DEINIT_SPLIT);
