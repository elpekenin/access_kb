// Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <quantum/xap/xap.h>
#include <quantum/split_common/transactions.h>
#include <tmk_core/protocol/usb_descriptor.h> // XAP_EPSIZE

#include "elpekenin/utils/compiler.h"


void reset_ee_slave(void);
void build_info_sync_keymap_callback(void);
void xap_execute_slave(const void *data);

typedef struct PACKED {
    uint8_t msg[XAP_EPSIZE];
} xap_split_msg_t;
_Static_assert(sizeof(xap_split_msg_t) == XAP_EPSIZE, "wrong size for xap_split_msg_t");
